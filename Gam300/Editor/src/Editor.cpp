#pragma warning(disable: 4834)  // Disable nodiscard warnings
#include "BoomEngine.h"
#include "Vendors/imgui/imgui.h"
#include "Windows/Inspector.h"
#include "Windows/Hierarchy.h"
#include "Windows/Resource.h"
#include "Windows/Viewport.h"
#include "Windows/MenuBar.h"
#include "Context/DebugHelpers.h"
using namespace Boom;

class Editor : public AppInterface
{
public:
    BOOM_INLINE Editor(ImGuiContext* imguiContext) : m_ImGuiContext(imguiContext)
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

    BOOM_INLINE void RenderHierarchy()
    {
        if (!m_ShowHierarchy) return;

        if (ImGui::Begin("Hierarchy", &m_ShowHierarchy)) {
            ImGui::Text("Scene Hierarchy");
            ImGui::Separator();

            if (ImGui::TreeNode("Scene Objects")) {
                if (ImGui::Selectable("Camera")) {
                    BOOM_INFO("Camera selected");
                }
                if (ImGui::Selectable("Light")) {
                    BOOM_INFO("Light selected");
                }
                if (ImGui::Selectable("Cube")) {
                    BOOM_INFO("Cube selected");
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();
    }

    BOOM_INLINE void RenderInspector()
    {
        if (!m_ShowInspector) return;

        if (ImGui::Begin("Inspector", &m_ShowInspector)) {
            ImGui::Text("Object Inspector");
            ImGui::Separator();

            if (ImGui::CollapsingHeader("Transform")) {
                static float pos[3] = { 0.0f, 0.0f, 0.0f };
                static float rot[3] = { 0.0f, 0.0f, 0.0f };
                static float scale[3] = { 1.0f, 1.0f, 1.0f };

                ImGui::DragFloat3("Position", pos, 0.1f);
                ImGui::DragFloat3("Rotation", rot, 1.0f, 0.0f, 360.0f);
                ImGui::DragFloat3("Scale", scale, 0.1f, 0.1f, 10.0f);
            }

            if (ImGui::CollapsingHeader("Renderer")) {
                static bool enabled = true;
                ImGui::Checkbox("Enabled", &enabled);

                static float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                ImGui::ColorEdit4("Color", color);
            }
        }
        ImGui::End();
    }

private:
    ImGuiContext* m_ImGuiContext = nullptr;
    bool m_ShowInspector = true;
    bool m_ShowHierarchy = true;
    bool m_ShowViewport = true;
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

        // Get the engine window handle
        GLFWwindow* engineWindow = app->GetWindowHandle();
        //BOOM_INFO("Got engine window handle: {}", (void*)engineWindow);

        ImGuiContext* imguiContext = nullptr;

        if (engineWindow) {
            // Make context current
            glfwMakeContextCurrent(engineWindow);
            GLFWwindow* current = glfwGetCurrentContext();

            if (current == engineWindow) {
                BOOM_INFO("Context is current, initializing ImGui...");

                // Initialize ImGui
                IMGUI_CHECKVERSION();
                imguiContext = ImGui::CreateContext();

                ImGuiIO& io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
                io.ConfigWindowsMoveFromTitleBarOnly = true;

                // Initialize backends
                bool platformInit = ImGui_ImplGlfw_InitForOpenGL(engineWindow, true);
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
            app->AttachLayer<Editor>(imguiContext);  // Use template version
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