#pragma once

#include <memory>

// Pull common engine/editor precompiled headers if you use one
#include "Editor/EditorPCH.h"

// Panel interfaces (included here so Editor.cpp can construct them)
#include "Panel/MenuBarPanel.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/ResourcePanel.h"
#include "Panels/DirectoryPanel.h"
#include "Panels/AudioPanel.h"
#include "Panels/PrefabBrowser.h"
#include "Panels/ViewportPanel.h"

namespace EditorUI {

    class Editor {
    public:
        Editor() = default;
        ~Editor() = default;

        // non-copyable / non-movable (keeps ownership simple)
        Editor(const Editor&) = delete;
        Editor& operator=(const Editor&) = delete;
        Editor(Editor&&) = delete;
        Editor& operator=(Editor&&) = delete;

        // lifecycle
        void Init();
        void Render();
        void Shutdown();

    private:
        // Panels (constructed in Init, rendered in Render)
        std::unique_ptr<MenuBarPanel>    m_MenuBar;
        std::unique_ptr<HierarchyPanel>  m_Hierarchy;
        std::unique_ptr<InspectorPanel>  m_Inspector;
        std::unique_ptr<ConsolePanel>    m_Console;
        std::unique_ptr<ResourcePanel>   m_Resources;
        std::unique_ptr<DirectoryPanel>  m_Directory;
        std::unique_ptr<AudioPanel>      m_Audio;
        std::unique_ptr<PrefabBrowser>   m_PrefabBrowser;
        std::unique_ptr<ViewportPanel>   m_Viewport;
    };

} // namespace EditorUI