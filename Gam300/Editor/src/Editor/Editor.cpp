// src/Editor/Editor.cpp
#include "Editor/Editor.h"

// ImGui + backends
#include "Vendors/imgui/imgui.h"
#include "Vendors/imgui/backends/imgui_impl_glfw.h"
#include "Vendors/imgui/backends/imgui_impl_opengl3.h"

// Gizmo
#include "ImGuizmo.h"

// GL (assumes your PCH sets this up; otherwise include your GL header)
#include <GLFW/glfw3.h>

namespace {

    // --- Helpers local to this translation unit ---------------------------------

    void BeginImguiFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
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

        ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        flags |= ImGuiWindowFlags_NoBackground;

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

// ------------------------ EditorUI::Editor -----------------------------------

namespace EditorUI {

    void Editor::Init()
    {
        // Panels are created here so they live across frames
        m_MenuBar = std::make_unique<MenuBarPanel>();
        m_Hierarchy = std::make_unique<HierarchyPanel>();
        m_Inspector = std::make_unique<InspectorPanel>();
        m_Console = std::make_unique<ConsolePanel>();
        m_Resources = std::make_unique<ResourcePanel>();
        m_Directory = std::make_unique<DirectoryPanel>();
        m_Audio = std::make_unique<AudioPanel>();
        m_PrefabBrowser = std::make_unique<PrefabBrowser>();
        m_Viewport = std::make_unique<ViewportPanel>();
    }

    void Editor::Render()
    {
        // Minimal GL state for ImGui draw pass (framebuffer 0; editor UI)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // Use the main viewport size for the GL viewport
        if (ImGuiViewport* vp = ImGui::GetMainViewport())
            glViewport(0, 0, (GLsizei)vp->Size.x, (GLsizei)vp->Size.y);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // --- Start frame ---
        BeginImguiFrame();

        // --- Layout root dockspace ---
        CreateMainDockSpace();

        // --- Panels (menu first, then windows) ---
        if (m_MenuBar)        m_MenuBar->Render();
        if (m_Viewport)       m_Viewport->Render();
        if (m_Hierarchy)      m_Hierarchy->Render();
        if (m_Inspector)      m_Inspector->Render();
        if (m_Resources)      m_Resources->Render();
        if (m_Directory)      m_Directory->Render();
        if (m_PrefabBrowser)  m_PrefabBrowser->Render();
        if (m_Console)        m_Console->Render();
        if (m_Audio)          m_Audio->Render();

        // --- End frame / draw ---
        EndImguiFrame();
    }

    void Editor::Shutdown()
    {
        // unique_ptr members clean up automatically
    }

} // namespace EditorUI
