// src/Editor/Editor.cpp
#include "Editor.h"

// Bring in the full AppContext definition here (not in the header)
#include "Context/Context.h"

// Panels (full definitions MUST be included here before ~Editor and method calls)
#include "Panels/MenuBarPanel.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/ResourcePanel.h"
#include "Panels/DirectoryPanel.h"
#include "Panels/AudioPanel.h"
#include "Panels/PrefabBrowserPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/PerformancePanel.h"
#include "Panels/PlaybackControlsPanel.h"

#include "BoomEngine.h"

// Gizmo
#include "ImGuizmo.h"

// GL loader must be included somewhere before GL calls.
// If your project uses glad/glew, include it *once* in a common cpp.
// Here we rely on your existing setup.
#include <GLFW/glfw3.h>
#include "Vendors/imgui/backends/imgui_impl_glfw.h"
#include "Vendors/imgui/backends/imgui_impl_opengl3.h"

namespace {

    // ---------------- Helpers local to this translation unit ----------------

    void BeginImguiFrame(ImGuiContext* ctx)
    {
        if (ctx) ImGui::SetCurrentContext(ctx);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ctx) ImGuizmo::SetImGuiContext(ctx);
        ImGuizmo::BeginFrame();
    }

    void EndImguiFrame()
    {
        ImGui::Render();
        if (ImDrawData* dd = ImGui::GetDrawData(); dd && dd->Valid)
        {
            ImGui_ImplOpenGL3_RenderDrawData(dd);
            glFlush();
        }
    }

    void CreateMainDockSpace()
    {
        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->Pos);
        ImGui::SetNextWindowSize(vp->Size);
        ImGui::SetNextWindowViewport(vp->ID);

        ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
            | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpace", nullptr, flags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspaceID = ImGui::GetID("MainDockspace");
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        ImGui::End();
    }

} // anonymous namespace

// ---------------------------- EditorUI::Editor ----------------------------

namespace EditorUI {

    Editor::Editor(ImGuiContext* imgui,
        entt::registry* registry,
        Boom::Application* app)
        : m_ImGuiContext(imgui), m_Registry(registry), m_App(app)
    {
    }

    // Define after all panel headers are included so the types are complete.
    Editor::~Editor() = default;

    void Editor::Init()
    {
        // Construct panels here; they persist across frames.
        // We pass `this` so panels can call owner->GetContext() etc.
        m_MenuBar = std::make_unique<MenuBarPanel>(this);
        m_Inspector = std::make_unique<InspectorPanel>(this);
        m_Hierarchy = std::make_unique<HierarchyPanel>(this);
        m_Console = std::make_unique<ConsolePanel>(this);
        m_Resources = std::make_unique<ResourcePanel>(this);
        m_Directory = std::make_unique<DirectoryPanel>(this);
        m_Audio = std::make_unique<AudioPanel>(this);
        m_PrefabBrowser = std::make_unique<PrefabBrowserPanel>(this);
        m_Viewport = std::make_unique<ViewportPanel>(this);
        m_Performance = std::make_unique<PerformancePanel>(this);
        m_Playback = std::make_unique<PlaybackControlsPanel>(this, m_App);

        // Panel-specific init
        if (m_Directory) m_Directory->Init();
    }
    void Editor::OnStart()
    {
        // AppInterface already filled m_Context before this call.
        // Make sure ImGui uses the context we created in main.cpp.
        if (m_ImGuiContext)
            ImGui::SetCurrentContext(m_ImGuiContext);

        BOOM_INFO("Editor::OnStart");
        Init();   // build all panels here (you already wrote this)
    }

    void Editor::OnUpdate()
    {
        // draw one ImGui frame of the editor
        Render(); // you already wrote this to NewFrame(), draw panels, RenderDrawData()
        // if (!m_ShowViewport) return;

        // if (ImGui::Begin("Viewport", &m_ShowViewport)) {

        //     // Show current scene info in menu bar
        //     ImGui::BeginTable("TextLayout", 2, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingFixedFit);
        //     ImGui::TableNextColumn();
        //     if (m_Application) {
        //         if (m_Application->IsSceneLoaded()) {
        //             std::string currentPath = m_Application->GetCurrentScenePath();
        //             if (!currentPath.empty()) {
        //                 // Extract just the filename
        //                 size_t lastSlash = currentPath.find_last_of("/\\");
        //                 std::string fileName = (lastSlash != std::string::npos) ?
        //                     currentPath.substr(lastSlash + 1) : currentPath;
        //                 ImGui::Text("Scene: %s", fileName.c_str());
        //             }
        //             else {
        //                 ImGui::Text("Scene: Unsaved");
        //             }
        //         }
        //         else {
        //             ImGui::Text("Scene: None");
        //         }

        //         ImGui::TableNextColumn();
        //         ImGui::Text("camera speed: %.2f", m_Context->window->camMoveMultiplier);
        //         if (m_Context->window->isShiftDown) {
        //             ImGui::SameLine();
        //             ImGui::Text("* %.2f", CONSTANTS::CAM_RUN_MULTIPLIER);
        //         }
        //         ImGui::EndTable();

        //         ImGui::Separator();
        //     }
            
        //     // Get frame texture from engine
        //     uint32_t frameTexture = GetSceneFrame();
        //     ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        //     //float aspectRatio = (viewportSize.y > 0) ? viewportSize.x / viewportSize.y : 1.0f;

        //     if (frameTexture > 0 && viewportSize.x > 0 && viewportSize.y > 0) {
        //         // Display the engine's rendered frame
        //         ImGui::Image((ImTextureID)(uintptr_t)frameTexture, viewportSize, ImVec2(0, 1), ImVec2(1, 0));  // Flipped UV for OpenGL

        //         m_Console.TrackLastItemAsViewport("Viewport");

        //         ImVec2 itemMin = ImGui::GetItemRectMin();
        //         ImVec2 itemMax = ImGui::GetItemRectMax();
        //         m_VP_TopLeft = itemMin;
        //         m_VP_Size = ImVec2(itemMax.x - itemMin.x, itemMax.y - itemMin.y);
        //         m_VP_Hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
        //         m_VP_Focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && m_VP_Hovered;

        //         // Convert ImGui screen-space rect -> GLFW window client-space rect
        //         ImVec2 mainPos = ImGui::GetMainViewport()->Pos; // top-left of GLFW client area in screen space
        //         double localX = (double)(m_VP_TopLeft.x - mainPos.x);
        //         double localY = (double)(m_VP_TopLeft.y - mainPos.y);
        //         double localW = (double)m_VP_Size.x;
        //         double localH = (double)m_VP_Size.y;

        //         // Allow camera look only when viewport is both focused and hovered (your policy)
        //         bool allowCamera = (m_VP_Hovered && m_VP_Focused);

        //         // Push region & permission to the window (each frame)
        //         m_Context->window->SetCameraInputRegion(localX, localY, localW, localH, allowCamera);

        //         if ((m_VP_Focused || m_VP_Hovered) && !ImGuizmo::IsUsing()) {
        //             // Top row
        //             if (ImGui::IsKeyPressed(ImGuiKey_1)) m_GizmoOperation = ImGuizmo::TRANSLATE;
        //             if (ImGui::IsKeyPressed(ImGuiKey_2)) m_GizmoOperation = ImGuizmo::ROTATE;
        //             if (ImGui::IsKeyPressed(ImGuiKey_3)) m_GizmoOperation = ImGuizmo::SCALE;

        //             // Numpad (optional)
        //             if (ImGui::IsKeyPressed(ImGuiKey_Keypad1)) m_GizmoOperation = ImGuizmo::TRANSLATE;
        //             if (ImGui::IsKeyPressed(ImGuiKey_Keypad2)) m_GizmoOperation = ImGuizmo::ROTATE;
        //             if (ImGui::IsKeyPressed(ImGuiKey_Keypad3)) m_GizmoOperation = ImGuizmo::SCALE;

        //             // Toggle local/world
        //             if (ImGui::IsKeyPressed(ImGuiKey_L))
        //                 m_gizmoMode = (m_gizmoMode == ImGuizmo::LOCAL) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
        //         }

        //         // Tooltip & debug
        //         if (m_VP_Hovered) {
        //             ImGui::SetTooltip("Engine Viewport - Scene render output");
        //         }
        //         static int debugCount = 0;
        //         if (++debugCount % 300 == 0) {
        //             BOOM_INFO("Viewport - Texture ID: {}, Size: {}x{}", frameTexture, viewportSize.x, viewportSize.y);
        //         }

        //         // Build camera matrices using the VIEWPORT aspect
        //         glm::mat4 cameraView(1.0f);
        //         glm::mat4 cameraProj(1.0f);
        //         {
        //             auto view = m_Context->scene.view<Boom::CameraComponent, Boom::TransformComponent>();
        //             if (view.begin() != view.end()) {
        //                 auto eid = view.front();
        //                 auto& camComp = view.get<Boom::CameraComponent>(eid);
        //                 auto& trans = view.get<Boom::TransformComponent>(eid);
        //                 cameraView = camComp.camera.View(trans.transform);
        //                 cameraProj = camComp.camera.Projection(m_Context->renderer->AspectRatio());
        //             }
        //         }

        //         // --------- GIZMO DRAW & MANIPULATE (inside viewport) ----------
        //         if (m_SelectedEntity != entt::null) {
        //             Boom::Entity selected{ &m_Context->scene, m_SelectedEntity };
        //             if (selected.Has<Boom::TransformComponent>()) {
        //                 auto& tc = selected.Get<Boom::TransformComponent>();
        //                 glm::mat4 model = tc.transform.Matrix();

        //                 // Set up ImGuizmo to draw in this window & rect only
        //                 ImGuizmo::SetOrthographic(false);
        //                 ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        //                 ImGuizmo::SetRect(m_VP_TopLeft.x, m_VP_TopLeft.y, m_VP_Size.x, m_VP_Size.y);

        //                 // Allow interaction only when hovered & focused
        //                 ImGuizmo::Enable(m_VP_Hovered && m_VP_Focused);

        //                 if (ImGuizmo::Manipulate(glm::value_ptr(cameraView),
        //                     glm::value_ptr(cameraProj),
        //                     m_GizmoOperation,
        //                     m_gizmoMode,
        //                     glm::value_ptr(model))) {
        //                     // Decompose back into TRS (ImGuizmo outputs degrees for rotation)
        //                     glm::vec3 t, rDeg, s;
        //                     ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model),
        //                         glm::value_ptr(t),
        //                         glm::value_ptr(rDeg),
        //                         glm::value_ptr(s));
        //                     // If your engine stores radians, convert rDeg -> radians here.
        //                     tc.transform.translate = t;
        //                     tc.transform.rotate = rDeg;   // or glm::radians(rDeg)
        //                     tc.transform.scale = s;
        //                 }
        //             }
        //         }
        //     }
        //     else {
        //         // Show debug info when texture is invalid
        //         ImGui::Text("Frame Texture ID: %u", frameTexture);
        //         ImGui::Text("Viewport Size: %.0fx%.0f", viewportSize.x, viewportSize.y);
        //         ImGui::Text("Waiting for engine frame data...");

        //         // Draw a placeholder
        //         ImDrawList* drawList = ImGui::GetWindowDrawList();
        //         ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        //         ImVec2 canvasSize = viewportSize;

        //         if (canvasSize.x > 50 && canvasSize.y > 50) {
        //             drawList->AddRectFilled(canvasPos,
        //                 ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
        //                 IM_COL32(64, 64, 64, 255));

        //             drawList->AddText(ImVec2(canvasPos.x + 10, canvasPos.y + 10),
        //                 IM_COL32(255, 255, 255, 255), "Engine Viewport");
        //         }

        //         m_VP_TopLeft = canvasPos;
        //         m_VP_Size = canvasSize;
        //         m_VP_Hovered = false;
        //         m_VP_Focused = false;
        //     }
        // }


        // ImGui::End();
    }

    void Editor::Render()
    {
        // Minimal GL state for editor UI pass
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (ImGuiViewport* vp = ImGui::GetMainViewport())
            glViewport(0, 0, (GLsizei)vp->Size.x, (GLsizei)vp->Size.y);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // --- Start frame ---
        BeginImguiFrame(m_ImGuiContext);

        // --- Layout root dockspace ---
        CreateMainDockSpace();

        // --- Panels (menu first, then windows) ---
        if (m_MenuBar)        m_MenuBar->Render();
        if (m_Viewport)       m_Viewport->Render();
        if (m_Hierarchy)      m_Hierarchy->Render();
        if (m_Inspector)      m_Inspector->Render();
        if (m_Resources)      m_Resources->OnShow();
        if (m_Directory)      m_Directory->OnShow();
        if (m_PrefabBrowser)  m_PrefabBrowser->Render();
        if (m_Console)        m_Console->Render();
        if (m_Audio)          m_Audio->Render();
        if (m_Performance)    m_Performance->Render();
        if (m_Playback)       m_Playback->OnShow();

        // --- End frame / draw ---
        EndImguiFrame();
    }

    void Editor::Shutdown()
    {
        // unique_ptr members will clean up automatically.
    }

} 
