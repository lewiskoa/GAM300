#pragma warning(disable: 4834)  // Disable nodiscard warnings
#
#include "BoomEngine.h"
#include "Vendors/imgui/imgui.h"
#include "Windows/Inspector.h"
#include "Windows/Hierarchy.h"
#include "Windows/Resource.h"
#include "Windows/Directory.h"
#include "Windows/Viewport.h"
#include "Windows/MenuBar.h"
#include "Windows/Console.h"
#include "Windows/AudioPanel.h"
#include "Context/DebugHelpers.h"
#include <glm/gtc/type_ptr.hpp>
#include "ImGuizmo.h"
#include "Context/Profiler.hpp"
#include "AppWindow.h"

using namespace Boom;
bool m_ShowPrefabBrowser = true;

class Editor : public AppInterface
{
public:
    // In your Editor class
public:
    BOOM_INLINE Editor(ImGuiContext* imguiContext, entt::registry* registry, Application* app)
        : m_ImGuiContext(imguiContext), m_Registry(registry), m_Application(app) // <-- Assign m_Registry here
    {
        BOOM_INFO("Editor created with ImGui context: {}", (void*)imguiContext);
    }

    BOOM_INLINE void OnStart() override
    {
        BOOM_INFO("Editor::OnStart - ImGui already initialized");

        // Set the context that was created in main()
        if (m_ImGuiContext) {
            ImGui::SetCurrentContext(m_ImGuiContext);
            BOOM_INFO("Editor::OnStart - Set ImGui context successfully");
        }

        m_Context->window->isEditor = true;

        LoadAllPrefabsFromDisk();

        RefreshSceneList(true);

        dw.Init();
    }

    BOOM_INLINE void OnUpdate() override
    {
        if (!m_ImGuiContext) return;

        SoundEngine::Instance().Update();

        // Make sure we're using the right context
        ImGui::SetCurrentContext(m_ImGuiContext);

        // Render editor UI
        RenderEditor();
    }

private:

    BOOM_INLINE void RenderEditor()
    {
        // Set up OpenGL state for ImGui
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, 1800, 900);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        ImGui::SetCurrentContext(m_ImGuiContext);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        if (m_AutoScanScenes) {
            m_ScanTimer += ImGui::GetIO().DeltaTime;
            if (m_ScanTimer >= m_ScanInterval) {
                m_ScanTimer = 0.0;
                RefreshSceneList(false);  // will only rebuild if changes detected
            }
        }

        // Handle keyboard shortcuts
        HandleKeyboardShortcuts();

        // Create the main editor layout
        CreateMainDockSpace();
        RenderMenuBar();
        RenderViewport();
        RenderHierarchy();
        RenderInspector();
        RenderPerformance();
        rw.OnShow();
        dw.OnShow();
        RenderPlaybackControls();
        RenderPrefabBrowser();
        if (m_ShowConsole)
            m_Console.OnShow();
        RenderAudioPanel();
        // Render scene management dialogs
        RenderSceneDialogs();
		RenderPrefabDialogs();


        // End frame and render
        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData && drawData->Valid) {
            ImGui_ImplOpenGL3_RenderDrawData(drawData);
            glFlush();
        }
    }

    BOOM_INLINE void CreateMainDockSpace() const
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        windowFlags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        windowFlags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpace", nullptr, windowFlags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspaceID = ImGui::GetID("MainDockspace");
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        ImGui::End();
    }

    BOOM_INLINE void RenderPlaybackControls()
    {
        if (!m_ShowPlaybackControls) return;

        if (ImGui::Begin("Playback Controls", &m_ShowPlaybackControls))
        {
            // Get the application reference (you'll need to store this)
           
			Application* app = m_Application;

            if (app) {
                ApplicationState currentState = app->GetState();

                // Display current state
                ImGui::Text("Application State: ");
                ImGui::SameLine();
                switch (currentState) {
                case ApplicationState::RUNNING:
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "RUNNING");
                    break;
                case ApplicationState::PAUSED:
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "PAUSED");
                    break;
                case ApplicationState::STOPPED:
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "STOPPED");
                    break;
                }

                ImGui::Separator();

                // Control buttons
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));

                // Play/Resume button
                bool canPlay = (currentState == ApplicationState::PAUSED || currentState == ApplicationState::STOPPED);
                if (!canPlay) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.6f, 0.0f, 1.0f));
                }

                if (ImGui::Button("Play/Resume", ImVec2(100, 30))) {
                    if (canPlay) {
                        app->Resume();
                        BOOM_INFO("[Editor] Play/Resume button clicked");
                    }
                }
                ImGui::PopStyleColor(3);

                ImGui::SameLine();

                // Pause button
                bool canPause = (currentState == ApplicationState::RUNNING);
                if (!canPause) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 0.0f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
                }

                if (ImGui::Button("Pause", ImVec2(100, 30))) {
                    if (canPause) {
                        app->Pause();
                        BOOM_INFO("[Editor] Pause button clicked");
                    }
                }
                ImGui::PopStyleColor(3);

                ImGui::SameLine();

                // Stop button
                bool canStop = (currentState != ApplicationState::STOPPED);
                if (!canStop) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
                }

                if (ImGui::Button("Stop", ImVec2(100, 30))) {
                    if (canStop) {
                        app->Stop();
                        BOOM_INFO("[Editor] Stop button clicked");
                    }
                }
                ImGui::PopStyleColor(3);

                ImGui::PopStyleVar();

                ImGui::Separator();

                // Additional info
                ImGui::Text("Keyboard Shortcuts:");
                ImGui::BulletText("Spacebar: Toggle Pause/Resume");
                ImGui::BulletText("Escape: Stop Application");

                // Time information
                if (currentState != ApplicationState::STOPPED) {
                    ImGui::Separator();
                    ImGui::Text("Adjusted Time: %.2f seconds", app->GetAdjustedTime());

                    if (currentState == ApplicationState::PAUSED) {
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Time is paused");
                    }
                }
            }
            else {
                ImGui::Text("Application reference not available");
            }
        }
        ImGui::End();
    }

    BOOM_INLINE void RenderMenuBar()
    {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                    if (m_Application) {
                        m_Application->NewScene("UntitledScene");
                        RefreshSceneList(true);
                        BOOM_INFO("[Editor] Created new scene");
                    }
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                    m_ShowSaveDialog = true;
                    // Set current scene name as default
                    if (m_Application && m_Application->IsSceneLoaded()) {
                        RefreshSceneList(true);
                        std::string currentPath = m_Application->GetCurrentScenePath();
                        if (!currentPath.empty()) {
                            // Extract scene name from path
                            size_t lastSlash = currentPath.find_last_of("/\\");
                            size_t lastDot = currentPath.find_last_of(".");
                            if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash) {
                                std::string sceneName = currentPath.substr(lastSlash + 1, lastDot - lastSlash - 1);
                                strncpy_s(m_SceneNameBuffer, sizeof(m_SceneNameBuffer), sceneName.c_str(), _TRUNCATE);
                            }
                        }
                    }
                }

                if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) {
                    m_ShowSaveDialog = true;
                    // Clear the buffer for new name
                    m_SceneNameBuffer[0] = '\0';
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Load Scene", "Ctrl+O")) {
                    m_ShowLoadDialog = true;
                    RefreshSceneList();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    if (m_Application) {
                        m_Application->Stop();
                    }
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Inspector", nullptr, &m_ShowInspector);
                ImGui::MenuItem("Hierarchy", nullptr, &m_ShowHierarchy);
                ImGui::MenuItem("Viewport", nullptr, &m_ShowViewport);
                ImGui::MenuItem("Prefab Browser", nullptr, &m_ShowPrefabBrowser);
                ImGui::MenuItem("Performance", nullptr, &m_ShowPerformance);
                ImGui::MenuItem("Playback Controls", nullptr, &m_ShowPlaybackControls);
                ImGui::MenuItem("Debug Console", nullptr, &m_ShowConsole);
                ImGui::MenuItem("Audio", nullptr, &m_ShowAudio);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Options")) {
                ImGui::MenuItem("Debug Draw", nullptr, &m_Context->renderer->isDrawDebugMode);
                ImGui::MenuItem("Normal View", nullptr, &m_Context->renderer->showNormalTexture);
                if (ImGui::BeginMenu("Low poly mode")) {
                    ImGui::Checkbox("Enabled", &m_Context->renderer->showLowPoly);
                    if (m_Context->renderer->showLowPoly) {
                        ImGui::SliderFloat("Dither Threshold", &m_Context->renderer->DitherThreshold(), 0.0f, 1.0f);
					}
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("GameObjects")) {
                if (ImGui::MenuItem("Create Empty Object")) {
                    Entity newEntity{ &m_Context->scene };
                    newEntity.Attach<InfoComponent>().name = "GameObject";
                    newEntity.Attach<TransformComponent>();
                    m_SelectedEntity = newEntity.ID();
                }

                if (ImGui::MenuItem("Create From Prefab...")) {
                    m_ShowPrefabBrowser = true;
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Save Selected as Prefab")) {
                    if (m_SelectedEntity != entt::null) {
                        m_ShowSavePrefabDialog = true;
                    }
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Delete Selected")) {
                    if (m_SelectedEntity != entt::null) {
                        m_Context->scene.destroy(m_SelectedEntity);
                        m_SelectedEntity = entt::null;
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    BOOM_INLINE void RenderPerformance()
    {
        if (!m_ShowPerformance) return;

        // normal dockable window
        if (ImGui::Begin("Performance", &m_ShowPerformance))
        {
            // timing
            ImGuiIO& io = ImGui::GetIO();
            const float fps = io.Framerate;
            const float ms = fps > 0.f ? 1000.f / fps : 0.f;

            ImGui::Text("FPS: %.1f  (%.2f ms)", fps, ms);
            ImGui::Separator();

            // history
            m_FpsHistory[m_FpsWriteIdx] = fps;
            m_FpsWriteIdx = (m_FpsWriteIdx + 1) % kPerfHistory;

            float tmp[kPerfHistory];
            for (int i = 0; i < kPerfHistory; ++i)
                tmp[i] = m_FpsHistory[(m_FpsWriteIdx + i) % kPerfHistory];

            // plot fills the panel width
            ImVec2 plotSize(ImGui::GetContentRegionAvail().x, 80.f);
            ImGui::PlotLines("FPS", tmp, kPerfHistory, 0, nullptr, 0.0f, 240.0f, plotSize);
           
            // simple status line
            if (fps >= 120.f) ImGui::TextColored(ImVec4(0.3f, 1, 0.3f, 1), "Very fast");
            else if (fps >= 60.f)  ImGui::TextColored(ImVec4(0.6f, 1, 0.6f, 1), "Good");
            else if (fps >= 30.f)  ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "Playable");
            else                   ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Slow");

            DrawProfilerPanel(m_Context->profiler);
        }
        ImGui::End();
    }

    
    BOOM_INLINE void RenderViewport()
    {
        if (!m_ShowViewport) return;

        if (ImGui::Begin("Viewport", &m_ShowViewport)) {

            // Show current scene info in menu bar
            ImGui::BeginTable("TextLayout", 2, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingFixedFit);
            ImGui::TableNextColumn();
            if (m_Application) {
                if (m_Application->IsSceneLoaded()) {
                    std::string currentPath = m_Application->GetCurrentScenePath();
                    if (!currentPath.empty()) {
                        // Extract just the filename
                        size_t lastSlash = currentPath.find_last_of("/\\");
                        std::string fileName = (lastSlash != std::string::npos) ?
                            currentPath.substr(lastSlash + 1) : currentPath;
                        ImGui::Text("Scene: %s", fileName.c_str());
                    }
                    else {
                        ImGui::Text("Scene: Unsaved");
                    }
                }
                else {
                    ImGui::Text("Scene: None");
                }

                ImGui::TableNextColumn();
                ImGui::Text("camera speed: %.2f", m_Context->window->camMoveMultiplier);
                if (m_Context->window->isShiftDown) {
                    ImGui::SameLine();
                    ImGui::Text("* %.2f", CONSTANTS::CAM_RUN_MULTIPLIER);
                }
                ImGui::EndTable();

                ImGui::Separator();
            }
            
            // Get frame texture from engine
            uint32_t frameTexture = GetSceneFrame();
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();
            float aspectRatio = (viewportSize.y > 0) ? viewportSize.x / viewportSize.y : 1.0f;

            if (frameTexture > 0 && viewportSize.x > 0 && viewportSize.y > 0) {
                // Display the engine's rendered frame
                ImGui::Image((ImTextureID)(uintptr_t)frameTexture, viewportSize,
                    ImVec2(0, 1), ImVec2(1, 0));  // Flipped UV for OpenGL

                m_Console.TrackLastItemAsViewport("Viewport");

                ImVec2 itemMin = ImGui::GetItemRectMin();
                ImVec2 itemMax = ImGui::GetItemRectMax();
                m_VP_TopLeft = itemMin;
                m_VP_Size = ImVec2(itemMax.x - itemMin.x, itemMax.y - itemMin.y);
                m_VP_Hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
                m_VP_Focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && m_VP_Hovered;

                // Convert ImGui screen-space rect -> GLFW window client-space rect
                ImVec2 mainPos = ImGui::GetMainViewport()->Pos; // top-left of GLFW client area in screen space
                double localX = (double)(m_VP_TopLeft.x - mainPos.x);
                double localY = (double)(m_VP_TopLeft.y - mainPos.y);
                double localW = (double)m_VP_Size.x;
                double localH = (double)m_VP_Size.y;

                // Allow camera look only when viewport is both focused and hovered (your policy)
                bool allowCamera = (m_VP_Hovered && m_VP_Focused);

                // Push region & permission to the window (each frame)
                m_Context->window->SetCameraInputRegion(localX, localY, localW, localH, allowCamera);

                if ((m_VP_Focused || m_VP_Hovered) && !ImGuizmo::IsUsing()) {
                    // Top row
                    if (ImGui::IsKeyPressed(ImGuiKey_1)) m_GizmoOperation = ImGuizmo::TRANSLATE;
                    if (ImGui::IsKeyPressed(ImGuiKey_2)) m_GizmoOperation = ImGuizmo::ROTATE;
                    if (ImGui::IsKeyPressed(ImGuiKey_3)) m_GizmoOperation = ImGuizmo::SCALE;

                    // Numpad (optional)
                    if (ImGui::IsKeyPressed(ImGuiKey_Keypad1)) m_GizmoOperation = ImGuizmo::TRANSLATE;
                    if (ImGui::IsKeyPressed(ImGuiKey_Keypad2)) m_GizmoOperation = ImGuizmo::ROTATE;
                    if (ImGui::IsKeyPressed(ImGuiKey_Keypad3)) m_GizmoOperation = ImGuizmo::SCALE;

                    // Toggle local/world
                    if (ImGui::IsKeyPressed(ImGuiKey_L))
                        m_gizmoMode = (m_gizmoMode == ImGuizmo::LOCAL) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
                }

                // Tooltip & debug
                if (m_VP_Hovered) {
                    ImGui::SetTooltip("Engine Viewport - Scene render output");
                }
                static int debugCount = 0;
                if (++debugCount % 300 == 0) {
                    BOOM_INFO("Viewport - Texture ID: {}, Size: {}x{}", frameTexture, viewportSize.x, viewportSize.y);
                }

                // Build camera matrices using the VIEWPORT aspect
                glm::mat4 cameraView(1.0f);
                glm::mat4 cameraProj(1.0f);
                {
                    auto view = m_Context->scene.view<Boom::CameraComponent, Boom::TransformComponent>();
                    if (view.begin() != view.end()) {
                        auto eid = view.front();
                        auto& camComp = view.get<Boom::CameraComponent>(eid);
                        auto& trans = view.get<Boom::TransformComponent>(eid);
                        cameraView = camComp.camera.View(trans.transform);
                        cameraProj = camComp.camera.Projection(aspectRatio);
                    }
                }

                // --------- GIZMO DRAW & MANIPULATE (inside viewport) ----------
                if (m_SelectedEntity != entt::null) {
                    Boom::Entity selected{ &m_Context->scene, m_SelectedEntity };
                    if (selected.Has<Boom::TransformComponent>()) {
                        auto& tc = selected.Get<Boom::TransformComponent>();
                        glm::mat4 model = tc.transform.Matrix();

                        // Set up ImGuizmo to draw in this window & rect only
                        ImGuizmo::SetOrthographic(false);
                        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
                        ImGuizmo::SetRect(m_VP_TopLeft.x, m_VP_TopLeft.y, m_VP_Size.x, m_VP_Size.y);

                        // Allow interaction only when hovered & focused
                        ImGuizmo::Enable(m_VP_Hovered && m_VP_Focused);

                        if (ImGuizmo::Manipulate(glm::value_ptr(cameraView),
                            glm::value_ptr(cameraProj),
                            m_GizmoOperation,
                            m_gizmoMode,
                            glm::value_ptr(model))) {
                            // Decompose back into TRS (ImGuizmo outputs degrees for rotation)
                            glm::vec3 t, rDeg, s;
                            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model),
                                glm::value_ptr(t),
                                glm::value_ptr(rDeg),
                                glm::value_ptr(s));
                            // If your engine stores radians, convert rDeg -> radians here.
                            tc.transform.translate = t;
                            tc.transform.rotate = rDeg;   // or glm::radians(rDeg)
                            tc.transform.scale = s;
                        }
                    }
                }
            }
            else {
                // Show debug info when texture is invalid
                ImGui::Text("Frame Texture ID: %u", frameTexture);
                ImGui::Text("Viewport Size: %.0fx%.0f", viewportSize.x, viewportSize.y);
                ImGui::Text("Waiting for engine frame data...");

                // Draw a placeholder
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 canvasPos = ImGui::GetCursorScreenPos();
                ImVec2 canvasSize = viewportSize;

                if (canvasSize.x > 50 && canvasSize.y > 50) {
                    drawList->AddRectFilled(canvasPos,
                        ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                        IM_COL32(64, 64, 64, 255));

                    drawList->AddText(ImVec2(canvasPos.x + 10, canvasPos.y + 10),
                        IM_COL32(255, 255, 255, 255), "Engine Viewport");
                }

                m_VP_TopLeft = canvasPos;
                m_VP_Size = canvasSize;
                m_VP_Hovered = false;
                m_VP_Focused = false;
            }
        }


        ImGui::End();
    }

    //Helpers
    void DrawComponentSection(
        const char* componentName,
        void* pComponent,
        const xproperty::type::object* (*getPropsFunc)(void*),
        bool canRemove,
        std::function<void()> removeFunc
    )
    {
        ImGui::PushID(componentName);

        bool isOpen = ImGui::CollapsingHeader(
            componentName,
            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap
        );

        // Context menu
        if (canRemove && ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Remove Component")) {
                if (removeFunc) removeFunc();
                ImGui::EndPopup();
                ImGui::PopID();
                return;
            }
            ImGui::EndPopup();
        }

        // Settings button
        if (canRemove) {
            ImGui::SameLine(ImGui::GetWindowWidth() - 30);
            if (ImGui::SmallButton("...")) {
                ImGui::OpenPopup("ComponentSettings");
            }

            if (ImGui::BeginPopup("ComponentSettings")) {
                if (ImGui::MenuItem("Remove Component")) {
                    if (removeFunc) removeFunc();
                    ImGui::EndPopup();
                    ImGui::PopID();
                    return;
                }
                ImGui::EndPopup();
            }
        }

        if (isOpen) {
            ImGui::Indent(12.0f);
            ImGui::Spacing();

            auto* props = getPropsFunc(pComponent);
            if (props) {
                DrawPropertiesUI(props, pComponent);
            }
            else {
                ImGui::TextDisabled("No properties available");
            }

            ImGui::Spacing();
            ImGui::Unindent(12.0f);
        }

        ImGui::PopID();
        ImGui::Spacing();
    }
    BOOM_INLINE void DrawPropertiesUI(const xproperty::type::object* pObj, void* pInstance)
    {
        xproperty::settings::context ctx;

        for (auto& member : pObj->m_Members) {
            DrawPropertyMember(member, pInstance, ctx);
        }
    }

    BOOM_INLINE void DrawPropertyMember(const xproperty::type::members& member, void* pInstance, xproperty::settings::context& ctx)
    {
        ImGui::PushID(member.m_pName);

        if (std::holds_alternative<xproperty::type::members::var>(member.m_Variant)) {
            auto& var = std::get<xproperty::type::members::var>(member.m_Variant);

            xproperty::any value;
            var.m_pRead(pInstance, value, var.m_UnregisteredEnumSpan, ctx);

            auto typeGUID = value.getTypeGuid();
            bool changed = false;

            void* pData = &value.m_Data;

            // Label column (Unity-style: label on left, control on right)
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%s", member.m_pName);
            ImGui::SameLine(150); // Fixed label width like Unity
            ImGui::SetNextItemWidth(-1); // Fill remaining width

            if (typeGUID == xproperty::settings::var_type<float>::guid_v) {
                float* pValue = reinterpret_cast<float*>(pData);
                changed = ImGui::DragFloat("##value", pValue, 0.01f);
            }
            else if (typeGUID == xproperty::settings::var_type<glm::vec3>::guid_v) {
                glm::vec3* pValue = reinterpret_cast<glm::vec3*>(pData);
                changed = ImGui::DragFloat3("##value", &pValue->x, 0.01f);
            }
            else if (typeGUID == xproperty::settings::var_type<int32_t>::guid_v) {
                int32_t* pValue = reinterpret_cast<int32_t*>(pData);
                changed = ImGui::DragInt("##value", pValue);
            }
            else if (typeGUID == xproperty::settings::var_type<uint64_t>::guid_v) {
                uint64_t* pValue = reinterpret_cast<uint64_t*>(pData);
                changed = ImGui::InputScalar("##value", ImGuiDataType_U64, pValue);
            }
            else if (typeGUID == xproperty::settings::var_type<std::string>::guid_v) {
                std::string* pValue = reinterpret_cast<std::string*>(pData);
                char buffer[256];
                strncpy_s(buffer, pValue->c_str(), sizeof(buffer));
                if (ImGui::InputText("##value", buffer, sizeof(buffer))) {
                    *pValue = std::string(buffer);
                    changed = true;
                }
            }
            else if (value.isEnum()) {
                const auto& enumSpan = value.getEnumSpan();
                const char* currentName = value.getEnumString();

                if (ImGui::BeginCombo("##value", currentName)) {
                    for (const auto& enumItem : enumSpan) {
                        bool selected = (enumItem.m_Value == value.getEnumValue());
                        if (ImGui::Selectable(enumItem.m_pName, selected)) {
                            xproperty::any newValue;
                            newValue.set<std::string>(enumItem.m_pName);
                            var.m_pWrite(pInstance, newValue, var.m_UnregisteredEnumSpan, ctx);
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            else {
                ImGui::TextDisabled("<unsupported>");
            }

            if (changed && !member.m_bConst && var.m_pWrite) {
                var.m_pWrite(pInstance, value, var.m_UnregisteredEnumSpan, ctx);
            }
        }
        else if (std::holds_alternative<xproperty::type::members::props>(member.m_Variant)) {
            auto& props = std::get<xproperty::type::members::props>(member.m_Variant);
            auto [pChild, pChildObj] = props.m_pCast(pInstance, ctx);

            if (pChild && pChildObj) {
                // Nested object with subtle indentation
                if (ImGui::TreeNodeEx(member.m_pName, ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Indent(8.0f);
                    for (auto& childMember : pChildObj->m_Members) {
                        DrawPropertyMember(childMember, pChild, ctx);
                    }
                    ImGui::Unindent(8.0f);
                    ImGui::TreePop();
                }
            }
        }

        ImGui::PopID();
    }

    // In your Editor class
    BOOM_INLINE void RenderHierarchy()
    {
        if (!m_ShowHierarchy) return;

        if (ImGui::Begin("Hierarchy", &m_ShowHierarchy)) {
            ImGui::Text("Scene Hierarchy");
            ImGui::Separator();

            // Use the correct scene registry from the AppContext
            auto view = m_Context->scene.view<Boom::InfoComponent>();

            for (auto entityID : view) {
                auto& info = view.get<Boom::InfoComponent>(entityID);

                // Compare the raw entity IDs for selection
                bool isSelected = (m_SelectedEntity == entityID);

                // Push the entity's unique ID onto ImGui's ID stack
                ImGui::PushID(static_cast<int>(entityID));

                // Now, you can safely use the (potentially non-unique) name for the label
                if (ImGui::Selectable(info.name.c_str(), isSelected)) {
                    // Assign the raw entity ID on click
                    m_SelectedEntity = entityID;
                }

                // Pop the ID off the stack to keep it clean for the next item
                ImGui::PopID();
            }
        }
        ImGui::End();
    }

    BOOM_INLINE void RenderInspector()
    {
        ImGui::Begin("Inspector");

        if (m_SelectedEntity != entt::null) {
            Boom::Entity selectedEntity{ &m_Context->scene, m_SelectedEntity };

            // ==================== ENTITY NAME ====================
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

            if (selectedEntity.Has<Boom::InfoComponent>()) {
                auto& info = selectedEntity.Get<Boom::InfoComponent>();

                ImGui::Text("Entity");
                ImGui::SameLine();

                ImGui::PushItemWidth(-1);
                char buffer[256];
                strncpy_s(buffer, sizeof(buffer), info.name.c_str(), sizeof(buffer) - 1);
                if (ImGui::InputText("##EntityName", buffer, sizeof(buffer))) {
                    info.name = std::string(buffer);
                }
                ImGui::PopItemWidth();
            }

            ImGui::PopStyleVar();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // ==================== COMPONENTS ====================

            // Transform Component (can't remove)
            if (selectedEntity.Has<Boom::TransformComponent>()) {
                auto& tc = selectedEntity.Get<Boom::TransformComponent>();
                DrawComponentSection("Transform", &tc, Boom::GetTransformComponentProperties, false, nullptr);
            }

            // Camera Component
            if (selectedEntity.Has<Boom::CameraComponent>()) {
                auto& cc = selectedEntity.Get<Boom::CameraComponent>();
                DrawComponentSection("Camera", &cc, Boom::GetCameraComponentProperties, true,
                    [&]() { m_Context->scene.remove<Boom::CameraComponent>(m_SelectedEntity); }
                );
            }

            // Model Component
            if (selectedEntity.Has<Boom::ModelComponent>()) {
                auto& mc = selectedEntity.Get<Boom::ModelComponent>();
                DrawComponentSection("Model Renderer", &mc, Boom::GetModelComponentProperties, true,
                    [&]() { m_Context->scene.remove<Boom::ModelComponent>(m_SelectedEntity); }
                );
            }

            // RigidBody Component
            if (selectedEntity.Has<Boom::RigidBodyComponent>()) {
                auto& rc = selectedEntity.Get<Boom::RigidBodyComponent>();
                DrawComponentSection("Rigidbody", &rc, Boom::GetRigidBodyComponentProperties, true,
                    [&]() { m_Context->scene.remove<Boom::RigidBodyComponent>(m_SelectedEntity); }
                );
            }

            // Collider Component
            if (selectedEntity.Has<Boom::ColliderComponent>()) {
                auto& col = selectedEntity.Get<Boom::ColliderComponent>();
                DrawComponentSection("Collider", &col, Boom::GetColliderComponentProperties, true,
                    [&]() { m_Context->scene.remove<Boom::ColliderComponent>(m_SelectedEntity); }
                );
            }

            //// Animator Component
            //if (selectedEntity.Has<Boom::AnimatorComponent>()) {
            //    auto& anim = selectedEntity.Get<Boom::AnimatorComponent>();
            //    DrawComponentSection("Animator", &anim, Boom::GetAnimatorComponentProperties, true,
            //        [&]() { m_Context->scene.remove<Boom::AnimatorComponent>(m_SelectedEntity); }
            //    );
            //}

            // Directional Light
            if (selectedEntity.Has<Boom::DirectLightComponent>()) {
                auto& dl = selectedEntity.Get<Boom::DirectLightComponent>();
                DrawComponentSection("Directional Light", &dl, Boom::GetDirectLightComponentProperties, true,
                    [&]() { m_Context->scene.remove<Boom::DirectLightComponent>(m_SelectedEntity); }
                );
            }

            // Point Light
            if (selectedEntity.Has<Boom::PointLightComponent>()) {
                auto& pl = selectedEntity.Get<Boom::PointLightComponent>();
                DrawComponentSection("Point Light", &pl, Boom::GetPointLightComponentProperties, true,
                    [&]() { m_Context->scene.remove<Boom::PointLightComponent>(m_SelectedEntity); }
                );
            }

            // Spot Light
            if (selectedEntity.Has<Boom::SpotLightComponent>()) {
                auto& sl = selectedEntity.Get<Boom::SpotLightComponent>();
                DrawComponentSection("Spot Light", &sl, Boom::GetSpotLightComponentProperties, true,
                    [&]() { m_Context->scene.remove<Boom::SpotLightComponent>(m_SelectedEntity); }
                );
            }

            // Skybox Component
            if (selectedEntity.Has<Boom::SkyboxComponent>()) {
                auto& sky = selectedEntity.Get<Boom::SkyboxComponent>();
                DrawComponentSection("Skybox", &sky, Boom::GetSkyboxComponentProperties, true,
                    [&]() { m_Context->scene.remove<Boom::SkyboxComponent>(m_SelectedEntity); }
                );
            }

            // ==================== ADD COMPONENT ====================
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Add Component", ImVec2(-1, 30))) {
                ImGui::OpenPopup("AddComponentPopup");
            }

            //if (ImGui::BeginPopup("AddComponentPopup")) {
            //    ImGui::Text("Add Component");
            //    ImGui::Separator();

            //    if (!selectedEntity.Has<Boom::CameraComponent>() && ImGui::Selectable("Camera")) {
            //        selectedEntity.Add<Boom::CameraComponent>();
            //    }
            //    if (!selectedEntity.Has<Boom::ModelComponent>() && ImGui::Selectable("Model Renderer")) {
            //        selectedEntity.Add<Boom::ModelComponent>();
            //    }
            //    if (!selectedEntity.Has<Boom::RigidBodyComponent>() && ImGui::Selectable("Rigidbody")) {
            //        selectedEntity.Add<Boom::RigidBodyComponent>();
            //    }
            //    if (!selectedEntity.Has<Boom::ColliderComponent>() && ImGui::Selectable("Collider")) {
            //        selectedEntity.Add<Boom::ColliderComponent>();
            //    }

            //    ImGui::EndPopup();
            //}
        }
        else {
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.5f - 20);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::TextWrapped("Select an entity in the hierarchy to view its properties");
            ImGui::PopStyleColor();
        }

        ImGui::End();
    }

    BOOM_INLINE void RefreshSceneList(bool force = false)
    {
        namespace fs = std::filesystem;

        if (!fs::exists(m_ScenesDir)) {
            BOOM_WARN("[Editor] '{}' directory doesn't exist, creating it...", m_ScenesDir);
            fs::create_directory(m_ScenesDir);
        }

        // Scan directory and build a new stamp map
        std::unordered_map<std::string, fs::file_time_type> newStamp;

        auto accept = [](const fs::path& p) {
            auto ext = p.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            if (ext != ".yaml" && ext != ".scene")
                return false;

            // ignore *_assets.yaml
            std::string stem = p.stem().string();
            if (stem.size() > 7 && stem.rfind("_assets") == stem.size() - 7)
                return false;

            return true;
            };

        for (const auto& entry : fs::directory_iterator(m_ScenesDir)) {
            if (!entry.is_regular_file()) continue;
            if (!accept(entry.path()))    continue;

            const std::string stem = entry.path().stem().string();  // scene name w/o extension
            newStamp[stem] = fs::last_write_time(entry.path());
        }

        // Detect changes (size mismatch or any (name,timestamp) mismatch)
        bool changed = force || (newStamp.size() != m_SceneStamp.size());
        if (!changed) {
            for (auto& [name, ts] : newStamp) {
                auto it = m_SceneStamp.find(name);
                if (it == m_SceneStamp.end() || it->second != ts) { changed = true; break; }
            }
        }
        if (!changed) return;

        // Rebuild the visible list (sorted)
        m_SceneStamp = std::move(newStamp);
        m_AvailableScenes.clear();
        m_AvailableScenes.reserve(m_SceneStamp.size());
        for (auto& [name, _] : m_SceneStamp) m_AvailableScenes.push_back(name);
        std::sort(m_AvailableScenes.begin(), m_AvailableScenes.end());

        // Keep selection valid
        if (m_SelectedSceneIndex >= (int)m_AvailableScenes.size()) m_SelectedSceneIndex = (int)m_AvailableScenes.size() - 1;
        if (m_SelectedSceneIndex < 0) m_SelectedSceneIndex = 0;

        BOOM_INFO("[Editor] Scene list refreshed ({} items).", (int)m_AvailableScenes.size());
    }

    BOOM_INLINE void RenderAudioPanel()
    {
        if (!m_ShowAudio) return;

        auto& audio = SoundEngine::Instance();

        // Your music catalog. Adjust names/paths to your project.
        static const std::vector<std::pair<std::string, std::string>> kTracks = {
            {"Menu",    "Resources/Audio/Fetty Wap.wav"},
            { "BOOM", "Resources/Audio/vboom.wav" },
            { "Fish", "Resources/Audio/FISH.wav" },
            { "Ambi", "Resources/Audio/outdoorAmbience.wav" },
            { "Schizo", "Resources/Audio/the voices.wav" },
        };

        // UI state
        static int  selected = 0;
        bool loop = false;
        static std::unordered_map<std::string, float> sVolume; // per-track volume

        for (auto& [name, _] : kTracks) if (!sVolume.count(name)) sVolume[name] = 1.0f;

        if (ImGui::Begin("Audio", &m_ShowAudio))
        {
            // Track select
            if (ImGui::BeginCombo("Track", kTracks[selected].first.c_str()))
            {
                for (int i = 0; i < (int)kTracks.size(); ++i) {
                    bool isSel = (i == selected);
                    if (ImGui::Selectable(kTracks[i].first.c_str(), isSel)) selected = i;
                    //if (isSel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            const std::string& name = kTracks[selected].first;
            const std::string& path = kTracks[selected].second;

            // Loop toggle (applies live if playing)
            if (ImGui::Checkbox("Loop", &loop)) {
                audio.SetLooping(name, loop);
            }
            ImGui::SameLine();
            if (ImGui::Button("Restart")) {
                audio.StopAllExcept("");                     // one music at a time
                audio.PlaySound(name, path, loop);
                audio.SetVolume(name, sVolume[name]);
            }

            // Volume
            float vol = sVolume[name];
            if (ImGui::SliderFloat("Volume", &vol, 0.0f, 1.0f, "%.2f")) {
                sVolume[name] = vol;
                audio.SetVolume(name, vol);
            }

            // Play / Stop / Pause
            const bool isPlaying = audio.IsPlaying(name);
            if (!isPlaying) {
                if (ImGui::Button("Play")) {
                    audio.StopAllExcept("");
                    audio.PlaySound(name, path, loop);
                    audio.SetVolume(name, sVolume[name]);
                }
            }
            else {
                if (ImGui::Button("Stop")) {
                    audio.StopSound(name);
                }
                ImGui::SameLine();
                static bool paused = false;
                if (ImGui::Checkbox("Paused", &paused)) {
                    audio.Pause(name, paused);
                }
            }

            // Quick switch buttons (optional)
            ImGui::SeparatorText("Quick Switch");
            for (int i = 0; i < (int)kTracks.size(); ++i) {
                ImGui::PushID(i);
                if (ImGui::Button(kTracks[i].first.c_str())) {
                    selected = i;
                    audio.StopAllExcept("");
                    audio.PlaySound(kTracks[i].first, kTracks[i].second, loop);
                    audio.SetVolume(kTracks[i].first, sVolume[kTracks[i].first]);
                }
                ImGui::PopID();
                if ((i % 3) != 2) ImGui::SameLine();
            }
        }
        ImGui::End();
    }

    BOOM_INLINE void RenderSceneDialogs()
    {
        // Save Scene Dialog
        if (m_ShowSaveDialog) {
            ImGui::OpenPopup("Save Scene");
            m_ShowSaveDialog = false; // Reset flag
        }

        if (ImGui::BeginPopupModal("Save Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter scene name:");
            ImGui::Separator();

            bool enterPressed = ImGui::InputText("##SceneName", m_SceneNameBuffer, sizeof(m_SceneNameBuffer),
                ImGuiInputTextFlags_EnterReturnsTrue);

            ImGui::Separator();

            bool saveClicked = ImGui::Button("Save", ImVec2(80, 0));
            ImGui::SameLine();
            bool cancelClicked = ImGui::Button("Cancel", ImVec2(80, 0));

            if ((saveClicked || enterPressed) && strlen(m_SceneNameBuffer) > 0) {
                if (m_Application) {
                    bool success = m_Application->SaveScene(std::string(m_SceneNameBuffer));
                    if (success) {
                        RefreshSceneList(true);
                        BOOM_INFO("[Editor] Scene '{}' saved successfully", m_SceneNameBuffer);
                    }
                    else {
                        BOOM_ERROR("[Editor] Failed to save scene '{}'", m_SceneNameBuffer);
                    }
                }
                ImGui::CloseCurrentPopup();
            }

            if (cancelClicked) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        // Load Scene Dialog
        if (m_ShowLoadDialog) {
            ImGui::OpenPopup("Load Scene");
            m_ShowLoadDialog = false; // Reset flag
        }

        if (ImGui::BeginPopupModal("Load Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Select scene to load:");
            ImGui::Separator();

            // Scene list
            if (m_AvailableScenes.empty()) {
                ImGui::Text("No scenes found in Scenes/ directory");
            }
            else {
                // Use a child window to contain the selectables and prevent popup closing
                if (ImGui::BeginChild("SceneList", ImVec2(250, 150), true)) {
                    for (int i = 0; i < (int)m_AvailableScenes.size(); i++) {
                        if (ImGui::Selectable(m_AvailableScenes[i].c_str(), m_SelectedSceneIndex == i)) {
                            m_SelectedSceneIndex = i;
                        }

                        // Double-click to load immediately
                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                            if (m_Application) {
                                std::string selectedScene = m_AvailableScenes[i];
                                bool success = m_Application->LoadScene(selectedScene);
                                if (success) {
                                    BOOM_INFO("[Editor] Scene '{}' loaded successfully", selectedScene);
                                    m_SelectedEntity = entt::null;
                                    RefreshSceneList(true);
                                }
                                else {
                                    BOOM_ERROR("[Editor] Failed to load scene '{}'", selectedScene);
                                }
                            }
                            ImGui::CloseCurrentPopup();
                            ImGui::EndChild();
                            ImGui::EndPopup();
                            return; // Exit early to avoid processing the rest
                        }
                    }
                }
                ImGui::EndChild();
            }

            ImGui::Separator();

            bool loadClicked = ImGui::Button("Load", ImVec2(80, 0));
            ImGui::SameLine();
            bool cancelClicked = ImGui::Button("Cancel", ImVec2(80, 0));

            if (loadClicked && m_SelectedSceneIndex >= 0 && m_SelectedSceneIndex < (int)m_AvailableScenes.size()) {
                if (m_Application) {
                    std::string selectedScene = m_AvailableScenes[m_SelectedSceneIndex];
                    bool success = m_Application->LoadScene(selectedScene);
                    if (success) {
                        BOOM_INFO("[Editor] Scene '{}' loaded successfully", selectedScene);
                        m_SelectedEntity = entt::null; // Clear selection when loading new scene
                    }
                    else {
                        BOOM_ERROR("[Editor] Failed to load scene '{}'", selectedScene);
                    }
                }
                ImGui::CloseCurrentPopup();
            }

            if (cancelClicked) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    BOOM_INLINE void RenderPrefabDialogs()
    {
        // Save Prefab Dialog
        if (m_ShowSavePrefabDialog) {
            ImGui::OpenPopup("Save as Prefab");
            m_ShowSavePrefabDialog = false;
        }

        if (ImGui::BeginPopupModal("Save as Prefab", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Save selected entity as prefab:");
            ImGui::Separator();

            bool enterPressed = ImGui::InputText("Prefab Name", m_PrefabNameBuffer, sizeof(m_PrefabNameBuffer),
                ImGuiInputTextFlags_EnterReturnsTrue);

            ImGui::Separator();

            bool saveClicked = ImGui::Button("Save", ImVec2(80, 0));
            ImGui::SameLine();
            bool cancelClicked = ImGui::Button("Cancel", ImVec2(80, 0));

            if ((saveClicked || enterPressed) && strlen(m_PrefabNameBuffer) > 0) {
                AssetID prefabID = RandomU64();
                auto prefab = PrefabUtility::CreatePrefabFromEntity(
                    *m_Context->assets,
                    prefabID,
                    std::string(m_PrefabNameBuffer),
                    m_Context->scene,
                    m_SelectedEntity
                );

                if (prefab) {
                    std::string filepath = "Prefabs/" + std::string(m_PrefabNameBuffer) + ".prefab";
                    bool saved = PrefabUtility::SavePrefab(*prefab, filepath);
                    if (saved) {
                        BOOM_INFO("[Editor] Saved prefab '{}'", m_PrefabNameBuffer);
                        RefreshPrefabList();
                    }
                }
                ImGui::CloseCurrentPopup();
            }

            if (cancelClicked) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        // Delete Prefab Dialog
        if (m_ShowDeletePrefabDialog) {
            ImGui::OpenPopup("Delete Prefab?");
            m_ShowDeletePrefabDialog = false;
            m_DeleteFromDisk = false; // Reset checkbox
        }

        if (ImGui::BeginPopupModal("Delete Prefab?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            auto& asset = m_Context->assets->Get<PrefabAsset>(m_PrefabToDelete);
            ImGui::Text("Delete prefab '%s'?", asset.name.c_str());
            ImGui::Spacing();

            ImGui::Checkbox("Delete from disk", &m_DeleteFromDisk);
            if (m_DeleteFromDisk) {
                ImGui::TextColored(ImVec4(1, 0.3f, 0, 1), "Warning: This cannot be undone!");
            }

            ImGui::Separator();

            if (ImGui::Button("Delete", ImVec2(120, 0))) {
                // SAVE the filepath BEFORE erasing the asset
                std::string filepath = "Prefabs/" + asset.name + ".prefab";
                std::string name = asset.name;

                // Remove from memory FIRST
                m_Context->assets->GetMap<PrefabAsset>().erase(m_PrefabToDelete);

                // NOW delete from disk (using the saved filepath)
                if (m_DeleteFromDisk) {
                    if (std::filesystem::exists(filepath)) {
                        std::filesystem::remove(filepath);
                        BOOM_INFO("[Editor] Deleted prefab file: {}", filepath);
                    }
                    else {
                        BOOM_WARN("[Editor] Prefab file not found: {}", filepath);
                    }
                }

                BOOM_INFO("[Editor] Deleted prefab '{}' from memory", name);
                RefreshPrefabList();

                if (m_SelectedPrefabID == m_PrefabToDelete) {
                    m_SelectedPrefabID = EMPTY_ASSET;
                }
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    BOOM_INLINE void RenderPrefabBrowser()
    {
        if (!m_ShowPrefabBrowser) return;

        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Prefab Browser", &m_ShowPrefabBrowser)) {

            // Toolbar
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));
            if (ImGui::Button("Refresh", ImVec2(80, 0))) {
                RefreshPrefabList();
                LoadAllPrefabsFromDisk();
            }
            ImGui::PopStyleColor();

            ImGui::SameLine();
            ImGui::TextDisabled("|");
            ImGui::SameLine();
            ImGui::Text("Prefabs: %d", (int)m_LoadedPrefabs.size());

            ImGui::Separator();

            // Search bar
            static char searchBuffer[256] = "";
            ImGui::SetNextItemWidth(-1);
            ImGui::InputTextWithHint("##Search", "Search prefabs...", searchBuffer, sizeof(searchBuffer));

            ImGui::Separator();

            // Prefab grid/list
            ImGui::BeginChild("PrefabList", ImVec2(0, -40), true);

            auto& prefabMap = m_Context->assets->GetMap<PrefabAsset>();
            std::string search = searchBuffer;
            std::transform(search.begin(), search.end(), search.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            int count = 0;
            for (auto& [uid, asset] : prefabMap) {
                if (uid == EMPTY_ASSET) continue;

                // Filter by search
                std::string name = asset->name;
                std::string nameLower = name;
                // FIX: Use lambda here too
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (!search.empty() && nameLower.find(search) == std::string::npos) {
                    continue;
                }

                count++;
                ImGui::PushID((int)uid);

                // Selectable prefab item
                bool selected = (m_SelectedPrefabID == uid);
                if (ImGui::Selectable(("## " + name).c_str(), selected, 0, ImVec2(0, 40))) {
                    m_SelectedPrefabID = uid;
                }

                // Double-click to instantiate
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                    EntityID newEntity = PrefabUtility::Instantiate(m_Context->scene, *m_Context->assets, uid);
                    if (newEntity != entt::null) {
                        m_SelectedEntity = newEntity;
                        BOOM_INFO("[Editor] Instantiated prefab '{}'", name);
                    }
                }

                // Right-click context menu
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Instantiate")) {
                        EntityID newEntity = PrefabUtility::Instantiate(m_Context->scene, *m_Context->assets, uid);
                        if (newEntity != entt::null) {
                            m_SelectedEntity = newEntity;
                        }
                    }
                    if (ImGui::MenuItem("Save to Disk")) {
                        std::string path = "Prefabs/" + name + ".prefab";
                        PrefabUtility::SavePrefab(*static_cast<PrefabAsset*>(asset.get()), path);
                        BOOM_INFO("[Editor] Saved prefab '{}'", name);
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Delete", nullptr, false, true)) {
                        m_PrefabToDelete = uid;
                        m_ShowDeletePrefabDialog = true;
                    }
                    ImGui::EndPopup();
                }

                // Draw the item content (same line as selectable)
                ImVec2 p = ImGui::GetItemRectMin();
                ImDrawList* draw = ImGui::GetWindowDrawList();

                // Icon placeholder
                ImVec2 iconMin = ImVec2(p.x + 5, p.y + 5);
                ImVec2 iconMax = ImVec2(p.x + 35, p.y + 35);
                draw->AddRectFilled(iconMin, iconMax, IM_COL32(80, 120, 180, 255), 4.0f);
                draw->AddText(ImVec2(iconMin.x + 8, iconMin.y + 8), IM_COL32(255, 255, 255, 255), "P");

                // Name
                draw->AddText(ImVec2(p.x + 45, p.y + 5), IM_COL32(255, 255, 255, 255), name.c_str());

                // Metadata
                char metaText[128];
                snprintf(metaText, sizeof(metaText), "ID: ...%llu", (unsigned long long)(uid % 100000));
                draw->AddText(ImVec2(p.x + 45, p.y + 22), IM_COL32(150, 150, 150, 255), metaText);

                ImGui::PopID();
            }

            if (count == 0) {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);
                ImGui::TextDisabled("No prefabs found");
                ImGui::TextDisabled("Create one via: GameObject > Save Selected as Prefab");
            }

            ImGui::EndChild();

            // Bottom toolbar
            ImGui::Separator();
            if (m_SelectedPrefabID != EMPTY_ASSET) {
                auto& asset = m_Context->assets->Get<PrefabAsset>(m_SelectedPrefabID);
                ImGui::Text("Selected: %s", asset.name.c_str());
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - 100);
                if (ImGui::Button("Instantiate", ImVec2(100, 0))) {
                    EntityID newEntity = PrefabUtility::Instantiate(m_Context->scene, *m_Context->assets, m_SelectedPrefabID);
                    if (newEntity != entt::null) {
                        m_SelectedEntity = newEntity;
                        BOOM_INFO("[Editor] Instantiated prefab '{}'", asset.name);
                    }
                }
            }
            else {
                ImGui::TextDisabled("No prefab selected");
            }
        }
        ImGui::End();
    }

    BOOM_INLINE void RefreshPrefabList()
    {
        m_LoadedPrefabs.clear();
        auto& prefabMap = m_Context->assets->GetMap<PrefabAsset>();
        for (auto& [uid, asset] : prefabMap) {
            if (uid != EMPTY_ASSET) {
                m_LoadedPrefabs.push_back({ asset->name, uid });
            }
        }
    }

    BOOM_INLINE void LoadAllPrefabsFromDisk()
    {
        namespace fs = std::filesystem;

        BOOM_INFO("[Editor] Starting to load prefabs from disk...");

        if (!fs::exists("Prefabs/")) {
            BOOM_WARN("[Editor] Prefabs directory doesn't exist, creating it...");
            fs::create_directory("Prefabs/");
            return;
        }

        BOOM_INFO("[Editor] Prefabs directory exists, scanning...");

        int loadedCount = 0;
        int fileCount = 0;

        for (const auto& entry : fs::directory_iterator("Prefabs/")) {
            fileCount++;
            std::string filepath = entry.path().string();
            std::string extension = entry.path().extension().string();

            BOOM_INFO("[Editor] Found file: {} (extension: {})", filepath, extension);

            if (entry.is_regular_file() && extension == ".prefab") {
                BOOM_INFO("[Editor] Attempting to load prefab: {}", filepath);

                AssetID prefabID = PrefabUtility::LoadPrefab(*m_Context->assets, filepath);

                if (prefabID != EMPTY_ASSET) {
                    loadedCount++;
                    BOOM_INFO("[Editor] Successfully loaded prefab ID: {}", prefabID);
                }
                else {
                    BOOM_ERROR("[Editor] Failed to load prefab from: {}", filepath);
                }
            }
        }

        BOOM_INFO("[Editor] Scanned {} files, loaded {} prefabs", fileCount, loadedCount);
        RefreshPrefabList();

        // Debug: Print what's in the registry
        auto& prefabMap = m_Context->assets->GetMap<PrefabAsset>();
        BOOM_INFO("[Editor] Prefabs in registry: {}", prefabMap.size() - 1); // -1 for EMPTY_ASSET
        for (auto& [uid, asset] : prefabMap) {
            if (uid != EMPTY_ASSET) {
                BOOM_INFO("[Editor]   - {} (ID: {})", asset->name, uid);
            }
        }
    }

    // Add keyboard shortcut handling:
    BOOM_INLINE void HandleKeyboardShortcuts()
    {
        ImGuiIO& io = ImGui::GetIO();

        // Scene management shortcuts
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N)) {
            // New Scene (Ctrl+N)
            if (m_Application) {
                m_Application->NewScene("UntitledScene");
                BOOM_INFO("[Editor] New scene created via shortcut");
            }
        }

        if (io.KeyCtrl && !io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_S)) {
            // Save Scene (Ctrl+S)
            if (m_Application && m_Application->IsSceneLoaded()) {
                std::string currentPath = m_Application->GetCurrentScenePath();
                if (!currentPath.empty()) {
                    // Extract scene name and save
                    size_t lastSlash = currentPath.find_last_of("/\\");
                    size_t lastDot = currentPath.find_last_of(".");
                    if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash) {
                        std::string sceneName = currentPath.substr(lastSlash + 1, lastDot - lastSlash - 1);
                        m_Application->SaveScene(sceneName);
                        BOOM_INFO("[Editor] Scene saved via shortcut");
                    }
                }
                else {
                    // No current scene, show save dialog
                    m_ShowSaveDialog = true;
                }
            }
        }

        if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_S)) {
            // Save Scene As (Ctrl+Shift+S)
            m_ShowSaveDialog = true;
        }

        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O)) {
            // Load Scene (Ctrl+O)
            m_ShowLoadDialog = true;
            RefreshSceneList();
        }
    }

private:
    ImGuiContext* m_ImGuiContext = nullptr;
    ConsoleWindow m_Console{ this };
    bool m_ShowConsole = true;
    bool m_ShowInspector = true;
    bool m_ShowHierarchy = true;
    bool m_ShowViewport = true;
    bool m_ShowPrefabBrowser = true;
    bool m_ShowAudio = true;
    bool m_ShowPerformance = true;
	bool m_ShowSavePrefabDialog = false;

    // Viewport State
    ImVec2 m_VP_TopLeft = { 0, 0 };
    ImVec2 m_VP_Size = { 0, 0 };
    bool   m_VP_Hovered = false;
    bool   m_VP_Focused = false;

	//prefab browser UI state
    char m_PrefabNameBuffer[256] = "NewPrefab";
    std::vector<std::pair<std::string, uint64_t>> m_LoadedPrefabs;
    AssetID m_SelectedPrefabID = EMPTY_ASSET;
    AssetID m_PrefabToDelete = EMPTY_ASSET;
    bool m_ShowDeletePrefabDialog = false;
    bool m_DeleteFromDisk = false;

    // Scene management UI state
    bool m_ShowSaveDialog = false;
    bool m_ShowLoadDialog = false;
    char m_SceneNameBuffer[256] = "NewScene";
    std::vector<std::string> m_AvailableScenes;
    int m_SelectedSceneIndex = 0;



    ImGuizmo::OPERATION m_GizmoOperation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE m_gizmoMode = ImGuizmo::WORLD;

    entt::registry* m_Registry = nullptr;
    entt::entity m_SelectedEntity = entt::null;

    static constexpr int kPerfHistory = 180;   // last ~3s @60 FPS
    float m_FpsHistory[kPerfHistory] = { 0.f };
    int   m_FpsWriteIdx = 0;

	Application* m_Application = nullptr; //To access application functions if needed
    bool m_ShowPlaybackControls = true;

    std::string m_ScenesDir = "Scenes";   // change if you use another folder
    bool        m_AutoScanScenes = true;  // toggle live refresh
    double      m_ScanInterval = 1.0;   // seconds
    double      m_ScanTimer = 0.0;   // accumulates delta time
    std::unordered_map<std::string, std::filesystem::file_time_type> m_SceneStamp;

    //remove when editor.cpp completed
    ResourceWindow rw{ this };
    DirectoryWindow dw{ this };
};

// Updated main function
int32_t main()
{
    try {
        MyEngineClass engine;
        engine.whatup();
        BOOM_INFO("Editor Started");

        if(!SoundEngine::Instance().Init()) {
            BOOM_ERROR("FMOD init failed");
            return -1;
        }

        // Create application
        auto app = engine.CreateApp();
        app->PostEvent<WindowTitleRenameEvent>("Boom Editor - Press 'Esc' to quit. 'WASD' to pan camera");
        entt::registry mainRegistry;

        // Get the engine window handle
        std::shared_ptr<GLFWwindow> engineWindow = app->GetWindowHandle();
        //BOOM_INFO("Got engine window handle: {}", (void*)engineWindow);

        ImGuiContext* imguiContext = nullptr;

        if (engineWindow) {
            // Make context current
            glfwMakeContextCurrent(engineWindow.get());
            GLFWwindow* current = glfwGetCurrentContext();

            if (current == engineWindow.get()) {
                BOOM_INFO("Context is current, initializing ImGui...");

                // Initialize ImGui
                IMGUI_CHECKVERSION();
                imguiContext = ImGui::CreateContext();

                ImGuiIO& io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
                io.ConfigWindowsMoveFromTitleBarOnly = true;

                // Initialize backends
                bool platformInit = ImGui_ImplGlfw_InitForOpenGL(engineWindow.get(), true);
                bool rendererInit = ImGui_ImplOpenGL3_Init("#version 450");

                if (platformInit && rendererInit) {
                    BOOM_INFO("ImGui initialized successfully!");
                    ImGui::StyleColorsDark();
                }
                else {
                    BOOM_ERROR("ImGui backend initialization failed");
                    ImGui::DestroyContext(imguiContext);
                    imguiContext = nullptr;
                }
            }
        }

        if (imguiContext) {
            // Create and attach editor with ImGui context
            app->AttachLayer<Editor>(imguiContext, &mainRegistry, app.get());
            // Use template version
        }
        else {
            BOOM_ERROR("Failed to initialize ImGui, running without editor");
        }

        // Run the application
        app->RunContext(true);

        // Cleanup ImGui
        if (imguiContext) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext(imguiContext);
        }
        SoundEngine::Instance().Shutdown();
    }
    catch (const std::exception& e) {
        BOOM_ERROR("Application failed: {}", e.what());
        return -1;
    }

    return 0;
}

