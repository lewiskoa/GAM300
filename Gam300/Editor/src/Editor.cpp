#pragma warning(disable: 4834)  // Disable nodiscard warnings
#
#include "BoomEngine.h"
#include "Vendors/imgui/imgui.h"
#include "Windows/Inspector.h"
#include "Windows/Hierarchy.h"
#include "Windows/Resource.h"
#include "Windows/Viewport.h"
#include "Windows/MenuBar.h"
#include "Windows/Console.h"
#include "Windows/AudioPanel.h"
#include "Context/DebugHelpers.h"
#include "Prefab/PrefabSystem.h"
#include <glm/gtc/type_ptr.hpp>
#include "ImGuizmo.h"
#include "Context/Profiler.hpp"

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

        // Handle keyboard shortcuts
        HandleKeyboardShortcuts();

        // Create the main editor layout
        CreateMainDockSpace();
        RenderMenuBar();
        RenderViewport();
        RenderHierarchy();
        RenderInspector();
        RenderGizmo();
        RenderPrefabBrowser();
        RenderPerformance();
        RenderResources();
        RenderPlaybackControls();
        if (m_ShowConsole)
            m_Console.OnShow(this);
        RenderAudioPanel();
        // Render scene management dialogs
        RenderSceneDialogs();


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
                        BOOM_INFO("[Editor] Created new scene");
                    }
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                    m_ShowSaveDialog = true;
                    // Set current scene name as default
                    if (m_Application && m_Application->IsSceneLoaded()) {
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
                ImGui::MenuItem("Debug Draw", nullptr, &m_Context->renderer->IsDrawDebugMode());
                ImGui::EndMenu();
            }

            // Show current scene info in menu bar
            if (m_Application) {
                ImGui::Separator();
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

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PREFAB_ASSET"))
                {
                    const char* prefabName = (const char*)payload->Data;
                    std::string path = "src/Prefabs/" + std::string(prefabName) + ".prefab";
                    std::cout << "Spawned prefab: " << prefabName << "\n";


                    // Instantiate prefab
                    entt::entity newEntity = Boom::PrefabSystem::InstantiatePrefab(
                        m_Context->scene,
                        *m_Context->assets,
                        path
                    );

                    if (newEntity != entt::null) {
                        std::cout << "Spawned prefab: " << prefabName << " (" << path << ")\n";
                    }
                    else {
                        std::cout << "Failed to spawn prefab: " << prefabName << "\n";
                    }
                }
                ImGui::EndDragDropTarget();
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

                glm::mat4 cameraProjection{};
                auto view = m_Context->scene.view<Boom::CameraComponent, Boom::TransformComponent>();
                if (view.begin() != view.end()) {
                    auto entityID = view.front();
                    auto& camComp = view.get<Boom::CameraComponent>(entityID);
                    cameraProjection = camComp.camera.Projection(aspectRatio);
                }

                // Add viewport interaction info
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Engine Viewport - Scene render output");
                }

                // Debug info (remove later)
                static int debugCount = 0;
                if (++debugCount % 300 == 0) {  // Every 5 seconds
                    BOOM_INFO("Viewport - Texture ID: {}, Size: {}x{}",
                        frameTexture, viewportSize.x, viewportSize.y);
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
            }
        }


        ImGui::End();
    }

    BOOM_INLINE void RenderGizmo()
    {
        // Do nothing if no entity is selected
        if (m_SelectedEntity == entt::null) {
            return;
        }

        // --- 1. Handle Keyboard Shortcuts to Change Operation ---
        if (ImGui::IsKeyPressed(ImGuiKey_W)) {
            m_GizmoOperation = ImGuizmo::TRANSLATE;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_E)) {
            m_GizmoOperation = ImGuizmo::ROTATE;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_R)) {
            m_GizmoOperation = ImGuizmo::SCALE;
        }

        // --- 2. Get Camera View and Projection Matrices ---
        glm::mat4 cameraView, cameraProjection;
        {
            int width, height;
            glfwGetWindowSize(m_Context->window->Handle().get(), &width, &height);
            float aspectRatio = (height > 0) ? (float)width / (float)height : 1.0f;

            auto view = m_Context->scene.view<Boom::CameraComponent, Boom::TransformComponent>();
            if (view.begin() != view.end()) { // Check if a camera exists
                auto entityID = view.front();
                auto& camComp = view.get<Boom::CameraComponent>(entityID);
                auto& transComp = view.get<Boom::TransformComponent>(entityID);
                cameraView = camComp.camera.View(transComp.transform);
                cameraProjection = camComp.camera.Projection(aspectRatio);
            }
            else {
                // Handle the case where no camera is found
                BOOM_WARN("No camera found in the scene for gizmo rendering.");
                // You might want to use a default or identity matrix here
                cameraView = glm::mat4(1.0f);
                cameraProjection = glm::mat4(1.0f);
            }
        }

        // --- 3. Prepare for Drawing the Gizmo ---
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetRect(ImGui::GetMainViewport()->Pos.x, ImGui::GetMainViewport()->Pos.y, ImGui::GetMainViewport()->Size.x, ImGui::GetMainViewport()->Size.y);

        // --- 4. Get the Selected Entity's Matrix ---
        Boom::Entity selectedEntity{ &m_Context->scene, m_SelectedEntity };
        auto& transformComp = selectedEntity.Get<Boom::TransformComponent>();
        glm::mat4 modelMatrix = transformComp.transform.Matrix();

        // --- 5. Draw the Gizmo and Update the Transform ---
        if (ImGuizmo::Manipulate(glm::value_ptr(cameraView),
            glm::value_ptr(cameraProjection),
            m_GizmoOperation,
            ImGuizmo::LOCAL,
            glm::value_ptr(modelMatrix)))
        {
            // If the user moved the gizmo, decompose the new matrix back into TRS
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMatrix),
                glm::value_ptr(transformComp.transform.translate),
                glm::value_ptr(transformComp.transform.rotate),
                glm::value_ptr(transformComp.transform.scale));
        }
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

// You will need #include <fstream> and #include <vector> if they aren't already in your Editor file.

    BOOM_INLINE void RenderPrefabBrowser()
    {
        if (!m_ShowPrefabBrowser) return;

        if (ImGui::Begin("Prefab Browser", &m_ShowPrefabBrowser)) {
            const char* prefabName = "Player"; // single prefab

            // Make it selectable and debug when clicked
            if (ImGui::Selectable(prefabName)) {
                std::cout << "[DEBUG] Prefab selected: " << prefabName << "\n";
                // Optional: store the currently selected prefab if needed
            }

            if (ImGui::Button("Spawn Player")) {
                Boom::PrefabSystem::InstantiatePrefab(
                    m_Context->scene,
                    *m_Context->assets,
                    "src/Prefab/Player.prefab"
                );
            }


            //// Drag source
            //if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            //    ImGui::SetDragDropPayload("PREFAB_ASSET", prefabName, strlen(prefabName) + 1);
            //    ImGui::Text("Dragging %s", prefabName);
            //    ImGui::EndDragDropSource();
            //}
        }
        ImGui::End();
    }

    BOOM_INLINE void RenderInspector()
    {
        ImGui::Begin("Inspector");

        // 1. Check if an entity is actually selected.
        //    entt::null is the default "invalid" entity ID.
        if (m_SelectedEntity != entt::null) {
            // Create the wrapper object to easily access components
            Boom::Entity selectedEntity{ &m_Context->scene, m_SelectedEntity };

            // --- Info Component ---
            if (selectedEntity.Has<Boom::InfoComponent>()) {
                auto& info = selectedEntity.Get<Boom::InfoComponent>(); 

                // ImGui expects a non-const char* for InputText
                char buffer[256];
                strncpy_s(buffer, sizeof(buffer), info.name.c_str(), sizeof(buffer) - 1);

                if (ImGui::InputText("##Name", buffer, sizeof(buffer))) {
                    info.name = std::string(buffer);
                }
            }

            ImGui::Separator();

            // --- Transform Component ---
            if (selectedEntity.Has<Boom::TransformComponent>()) {
                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto& tc = selectedEntity.Get<Boom::TransformComponent>();

                    // Use DragFloat3 for position, rotation, and scale
                    ImGui::DragFloat3("Position", glm::value_ptr(tc.transform.translate), 0.1f);
                    ImGui::DragFloat3("Rotation", glm::value_ptr(tc.transform.rotate), 1.0f);
                    ImGui::DragFloat3("Scale", glm::value_ptr(tc.transform.scale), 0.1f);
                }
            }

            // --- Model Component ---
            if (selectedEntity.Has<Boom::ModelComponent>()) {
                if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto& mc = selectedEntity.Get<Boom::ModelComponent>();
                    ImGui::Text("Model ID: %llu", mc.modelID);
                    ImGui::Text("Material ID: %llu", mc.materialID);
                }
            }

            // Add more "if (selectedEntity.Has<...>()" blocks for other components...

        }
        else {
            // 2. If no entity is selected, show this message.
            ImGui::Text("No entity selected.");
        }

        ImGui::End();
    }
    
    BOOM_INLINE void RenderResources() {
        rw.OnShow(this);
    }

    BOOM_INLINE void RefreshSceneList()
    {
        m_AvailableScenes.clear();

        // For now, add some default scenes - you can implement directory scanning later
        m_AvailableScenes.push_back("default");
        m_AvailableScenes.push_back("test");
        //m_AvailableScenes.push_back("demo_level");

        // TODO: Implement actual directory scanning for .yaml files in Scenes/ folder
        // This would require platform-specific code or a library like std::filesystem
    }

    BOOM_INLINE void RenderAudioPanel()
    {
        if (!m_ShowAudio) return;

        auto& audio = SoundEngine::Instance();

        // Your music catalog. Adjust names/paths to your project.
        static const std::vector<std::pair<std::string, std::string>> kTracks = {
            {"Menu",    "Assets/Audio/Music/menu_theme.ogg"},
            {"Level 1", "Assets/Audio/Music/level1_loop.ogg"},
            {"Boss",    "Assets/Audio/Music/boss_bgm.ogg"},
            {"Credits", "Assets/Audio/Music/credits.ogg"},
        };

        // UI state
        static int  selected = 0;
        static bool loop = true;
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
                    if (isSel) ImGui::SetItemDefaultFocus();
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


    //remove when editor.cpp completed
    ResourceWindow rw{this};
};

// Updated main function
int32_t main()
{
    try {
        MyEngineClass engine;
        engine.whatup();
        BOOM_INFO("Editor Started");

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

