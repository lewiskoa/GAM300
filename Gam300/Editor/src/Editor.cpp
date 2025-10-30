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

    Editor::Editor(Boom::AppContext* ctx, ImGuiContext* imgui)
        : m_Context(ctx), m_ImGuiContext(imgui)
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
        m_Playback = std::make_unique<PlaybackControlsPanel>(this);

        // Panel-specific init
        if (m_Directory) m_Directory->Init();
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
        if (m_Console)        m_Console->OnShow();
        if (m_Audio)          m_Audio->Render();
        if (m_Performance)    m_Performance->Render();
        if (m_Playback)       m_Playback->Render();

        // --- End frame / draw ---
        EndImguiFrame();
    }

    void Editor::Shutdown()
    {
        // unique_ptr members will clean up automatically.
    }

} 
