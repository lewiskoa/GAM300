#pragma warning(disable: 4834)  // Disable nodiscard warnings
#include "BoomEngine.h"
#include "Vendors/imgui/imgui.h"
#include "Windows/Inspector.h"
#include "Windows/Hierarchy.h"
#include "Windows/Resource.h"
#include "Windows/Viewport.h"
#include "Windows/MenuBar.h"
#include "Context/DebugHelpers.h"
#include "Prefab/PrefabSystem.h"
#include "ImGuizmo.h"
#include <glm/gtc/type_ptr.hpp>


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

        // Create the main editor layout
        CreateMainDockSpace();
        RenderMenuBar();
        RenderViewport();
        RenderHierarchy();
        RenderInspector();
        RenderPrefabBrowser();

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
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    BOOM_INLINE void RenderViewport()
    {
        if (!m_ShowViewport) return;

        if (ImGui::Begin("Viewport", &m_ShowViewport)) {
            // Get frame texture from engine
            uint32_t frameTexture = GetSceneFrame();
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();

            if (frameTexture > 0 && viewportSize.x > 0 && viewportSize.y > 0) {
                // Display the engine's rendered frame
                ImGui::Image((ImTextureID)(uintptr_t)frameTexture, viewportSize,
                    ImVec2(0, 1), ImVec2(1, 0));  // Flipped UV for OpenGL

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
        // Make sure an entity is selected and has a transform component
        if (!m_Registry->valid(m_SelectedEntity) || !m_Registry->all_of<TransformComponent>(m_SelectedEntity)) {
            return;
        }

        // Must be called once per frame
        ImGuizmo::BeginFrame();

        // Set the area of the viewport where the gizmo will be drawn
        // This assumes you are calling this function inside your Viewport's ImGui::Begin()/End() block
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

        glm::mat4 cameraView = glm::mat4(1.f);       // Default to identity matrix
        glm::mat4 cameraProjection = glm::mat4(1.f); // Default to identity matrix

        // Create a view of all entities that have BOTH a CameraComponent AND a TransformComponent
        auto cameraEntities = m_Registry->view<CameraComponent, TransformComponent>();
        if (cameraEntities.size_hint() > 0) {
            // Get the very first entity from that view
            entt::entity mainCameraEntity = cameraEntities.front();

            // Get that entity's components
            auto& cameraComponent = m_Registry->get<CameraComponent>(mainCameraEntity);
            auto& transformComponent = m_Registry->get<TransformComponent>(mainCameraEntity);

            // 2. Call the existing methods from your Camera3D struct
            float aspectRatio = ImGui::GetWindowWidth() / ImGui::GetWindowHeight();
            cameraView = cameraComponent.camera.View(transformComponent.transform);
            cameraProjection = cameraComponent.camera.Projection(aspectRatio);
        }

        // Get the transform component and build the model matrix for the selected entity
        auto& tc = m_Registry->get<TransformComponent>(m_SelectedEntity);
        glm::mat4 modelMatrix = tc.transform.Matrix(); // Assumes your Transform3D has a Matrix() function

        // Call the main gizmo function
        ImGuizmo::Manipulate(
            glm::value_ptr(cameraView),
            glm::value_ptr(cameraProjection),
            m_gizmoOperation, // Use the operation from our member variable
            m_gizmoMode,
            glm::value_ptr(modelMatrix)
        );

        // If the user is interacting with the gizmo...
        if (ImGuizmo::IsUsing())
        {
            // Decompose the modified model matrix back into TRS components
            glm::vec3 newTranslation, newRotation, newScale;
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMatrix), glm::value_ptr(newTranslation), glm::value_ptr(newRotation), glm::value_ptr(newScale));

            // Update your entity's transform component with the new values
            tc.transform.translate = newTranslation;
            tc.transform.rotate = newRotation;
            tc.transform.scale = newScale;
        }
    }

    // In your Editor class
    BOOM_INLINE void RenderHierarchy()
    {
        if (!m_ShowHierarchy) return;

        if (ImGui::Begin("Hierarchy", &m_ShowHierarchy)) {

            if (ImGui::Button("Translate")) m_gizmoOperation = ImGuizmo::TRANSLATE;
            ImGui::SameLine();
            if (ImGui::Button("Rotate")) m_gizmoOperation = ImGuizmo::ROTATE;
            ImGui::SameLine();
            if (ImGui::Button("Scale")) m_gizmoOperation = ImGuizmo::SCALE;
            ImGui::Separator();

            ImGui::Text("Scene Hierarchy");
            ImGui::Separator();

            // This is the new, correct way to loop through all entities
            for (auto entityID : m_Registry->view<entt::entity>()) {

                std::string name = "Entity " + std::to_string(static_cast<uint32_t>(entityID));

                if (ImGui::Selectable(name.c_str(), m_SelectedEntity == entityID)) {
                    m_SelectedEntity = entityID;
                }
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
                // To add a new prefab, you must add its name to this list
                // and then recompile your program.
            };
            // ==========================================================

            ImGui::Text("Available Prefabs:");
            ImGui::Separator();

            // Loop through the hardcoded array of names
            for (const char* prefabName : prefabNames) {
                if (ImGui::Button(prefabName)) {
                    // When clicked, create the full path and instantiate it
                    std::string fullPath = "assets/prefabs/" + std::string(prefabName) + ".prefab";
                    Boom::InstantiatePrefab(*m_Registry, fullPath);
                }
            }
        }
        ImGui::End();
    }

    BOOM_INLINE void RenderInspector()
    {
        if (!m_ShowInspector) return;

        if (ImGui::Begin("Inspector", &m_ShowInspector)) {
            // First, check if a valid entity is selected
            if (m_Registry->valid(m_SelectedEntity)) {

                // --- UI to display components ---
                // Example for TransformComponent
                if (m_Registry->all_of<TransformComponent>(m_SelectedEntity)) {
                    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                        auto& tc = m_Registry->get<TransformComponent>(m_SelectedEntity);
                        ImGui::DragFloat3("Translate", &tc.transform.translate.x, 0.1f);
                        ImGui::DragFloat3("Rotate", &tc.transform.rotate.x, 1.0f);
                        ImGui::DragFloat3("Scale", &tc.transform.scale.x, 0.1f);
                    }
                }
                // (You can add more `if` blocks here to display other components)


                // --- "Save as Prefab" Button Logic ---
                ImGui::Separator();
                if (ImGui::Button("Save as Prefab")) {
                    ImGui::OpenPopup("SavePrefabPopup");
                }

                if (ImGui::BeginPopup("SavePrefabPopup")) {
                    static char prefabName[128] = "NewPrefab";
                    ImGui::InputText("Prefab Name", prefabName, 128);
                    if (ImGui::Button("Save")) {
                        // Call the save function from your PrefabSystem.h
                        Boom::SaveEntityAsPrefab(*m_Registry, m_SelectedEntity, std::string(prefabName));
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }
            else {
                ImGui::Text("No entity selected.");
            }
        }
        ImGui::End();
    }

private:
    ImGuiContext* m_ImGuiContext = nullptr;
    bool m_ShowInspector = true;
    bool m_ShowHierarchy = true;
    bool m_ShowViewport = true;


    ImGuizmo::OPERATION m_gizmoOperation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE m_gizmoMode = ImGuizmo::WORLD;

    entt::registry* m_Registry = nullptr;
    entt::entity m_SelectedEntity = entt::null;
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