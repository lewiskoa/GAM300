#pragma warning(disable: 4834)  // Disable nodiscard warnings
#
#include "BoomEngine.h"
#include "Vendors/imgui/imgui.h"
#include "Windows/Inspector.h"
#include "Windows/Hierarchy.h"
#include "Windows/Resource.h"
#include "Windows/Viewport.h"
#include "Windows/MenuBar.h"
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
    BOOM_INLINE Editor(ImGuiContext* imguiContext, entt::registry* registry)
        : m_ImGuiContext(imguiContext), m_Registry(registry) // <-- Assign m_Registry here
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

        // Create the main editor layout
        CreateMainDockSpace();
        RenderMenuBar();
        RenderViewport();
        RenderHierarchy();
        RenderInspector();
        RenderGizmo();
        RenderPrefabBrowser();
        RenderPerformance();
		
        // End frame and render
        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData && drawData->Valid) {
            ImGui_ImplOpenGL3_RenderDrawData(drawData);
            glFlush();
        }
    }

    BOOM_INLINE void CreateMainDockSpace()
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

    BOOM_INLINE void RenderMenuBar()
    {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Scene")) {
                    BOOM_INFO("New Scene selected");
                }
                if (ImGui::MenuItem("Open Scene")) {
                    BOOM_INFO("Open Scene selected");
                }
                if (ImGui::MenuItem("Save Scene")) {
                    BOOM_INFO("Save Scene selected");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    BOOM_INFO("Exit selected");
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Inspector", nullptr, &m_ShowInspector);
                ImGui::MenuItem("Hierarchy", nullptr, &m_ShowHierarchy);
                ImGui::MenuItem("Viewport", nullptr, &m_ShowViewport);
                ImGui::MenuItem("Prefab Browser", nullptr, &m_ShowPrefabBrowser); // <-- MAKE SURE THIS LINE IS HERE
                ImGui::MenuItem("Performance", nullptr, &m_ShowPerformance);
			
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Options")) {
                ImGui::MenuItem("Debug Draw", nullptr, &m_Context->renderer->IsDrawDebugMode());
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
            // Get frame texture from engine
            uint32_t frameTexture = GetSceneFrame();
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();

            float aspectRatio = (viewportSize.y > 0) ? viewportSize.x / viewportSize.y : 1.0f;

            if (frameTexture > 0 && viewportSize.x > 0 && viewportSize.y > 0) {
                // Display the engine's rendered frame
                ImGui::Image((ImTextureID)(uintptr_t)frameTexture, viewportSize,
                    ImVec2(0, 1), ImVec2(1, 0));  // Flipped UV for OpenGL

                glm::mat4 cameraProjection;
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
            // ==========================================================
            // ===== THE PREFAB LIST IS NOW HARDCODED IN THE CODE =====
            // ==========================================================
            const char* prefabNames[] = {
                "Player",
                "RedBarrel",
                "EnemyGrunt"

            };
            // ==========================================================

            ImGui::Text("Available Prefabs:");
            ImGui::Separator();

            // Loop through the hardcoded array of names
            for (const char* prefabName : prefabNames) {
                // Use Selectable instead of Button for a cleaner drag source
                ImGui::Selectable(prefabName);

                // Check if the item is being dragged
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    // Set the payload with a unique identifier and the prefab name
                    ImGui::SetDragDropPayload("PREFAB_ASSET", prefabName, strlen(prefabName) + 1);

                    // Optional: display a tooltip while dragging
                    ImGui::Text("Dragging %s", prefabName);

                    ImGui::EndDragDropSource();
                }
            }
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

private:
    ImGuiContext* m_ImGuiContext = nullptr;
    bool m_ShowInspector = true;
    bool m_ShowHierarchy = true;
    bool m_ShowViewport = true;
    bool m_ShowPrefabBrowser = true;
    bool m_ShowPerformance = true;
	

    ImGuizmo::OPERATION m_GizmoOperation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE m_gizmoMode = ImGuizmo::WORLD;

    entt::registry* m_Registry = nullptr;
    entt::entity m_SelectedEntity = entt::null;

    static constexpr int kPerfHistory = 180;   // last ~3s @60 FPS
    float m_FpsHistory[kPerfHistory] = { 0.f };
    int   m_FpsWriteIdx = 0;
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
            app->AttachLayer<Editor>(imguiContext, &mainRegistry);
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

    }
    catch (const std::exception& e) {
        BOOM_ERROR("Application failed: {}", e.what());
        return -1;
    }

    return 0;
}

