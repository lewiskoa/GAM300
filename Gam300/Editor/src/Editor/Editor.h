#pragma once
#include <memory>

// Keep heavy headers out of here to avoid cycles.
// Just forward-declare the few types we need.
struct ImGuiContext;            // ImGui is global-namespace

namespace Boom { struct AppContext; }  // your engine context

namespace EditorUI {

    // Forward-declare all panels (their full defs live in each panel header).
    class MenuBarPanel;
    class HierarchyPanel;
    class InspectorPanel;
    class ConsolePanel;
    class ResourcePanel;
    class DirectoryPanel;
    class AudioPanel;
    class PrefabBrowserPanel;
    class ViewportPanel;
    class PerformancePanel;
    class PlaybackControlsPanel;

    class Editor {
    public:
        explicit Editor(Boom::AppContext* ctx, ImGuiContext* imgui = nullptr);
        ~Editor();                      // defined in .cpp after including all panels

        Editor(const Editor&) = delete;
        Editor& operator=(const Editor&) = delete;
        Editor(Editor&&) = delete;
        Editor& operator=(Editor&&) = delete;

        // lifecycle
        void Init();
        void Render();
        void Shutdown();

        // accessors used by panels
        Boom::AppContext* GetContext() const { return m_Context; }
        ImGuiContext* GetImGuiContext() const { return m_ImGuiContext; }

    private:
        Boom::AppContext* m_Context = nullptr;
        ImGuiContext* m_ImGuiContext = nullptr;

        // Panels (constructed in Init, destroyed in ~Editor)
        std::unique_ptr<MenuBarPanel>           m_MenuBar;
        std::unique_ptr<HierarchyPanel>         m_Hierarchy;
        std::unique_ptr<InspectorPanel>         m_Inspector;
        std::unique_ptr<ConsolePanel>           m_Console;
        std::unique_ptr<ResourcePanel>          m_Resources;
        std::unique_ptr<DirectoryPanel>         m_Directory;
        std::unique_ptr<AudioPanel>             m_Audio;
        std::unique_ptr<PrefabBrowserPanel>     m_PrefabBrowser;
        std::unique_ptr<ViewportPanel>          m_Viewport;
        std::unique_ptr<PerformancePanel>       m_Performance;
        std::unique_ptr<PlaybackControlsPanel>  m_Playback;
    };

} // namespace EditorUI
