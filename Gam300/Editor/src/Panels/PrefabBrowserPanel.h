#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include "Vendors/imgui/imgui.h"
#include <entt/entity/entity.hpp> // entt::entity

namespace Boom { struct AppContext; }

namespace EditorUI {

    class Editor; // forward-declare, full type only needed in .cpp

    class PrefabBrowserPanel {
    public:
        explicit PrefabBrowserPanel(Editor* owner);

        // Editor.cpp calls this
        void Render();

        // External toggles (optional)
        void Show(bool v) { m_ShowPrefabBrowser = v; }
        bool IsVisible() const { return m_ShowPrefabBrowser; }

        // Dialog triggers (optional)
        void OpenSaveDialog() { m_ShowSavePrefabDialog = true; }
        void OpenDeleteDialog(std::uint64_t id) { m_PrefabToDelete = id; m_ShowDeletePrefabDialog = true; }

    private:
        // UI sections
        void OnShow();                // internal render
        void RenderPrefabDialogs();
        void RenderPrefabBrowser();

        // Data updates
        void RefreshPrefabList();
        void LoadAllPrefabsFromDisk(); // scans Prefabs/ and loads .prefab assets

    private:
        // Owner/context
        Editor* m_Owner = nullptr;    // non-owning
        Boom::AppContext* m_Ctx = nullptr;    // cached from Editor

        // Visibility
        bool m_ShowPrefabBrowser = true;

        // Dialog states
        bool m_ShowSavePrefabDialog = false;
        bool m_ShowDeletePrefabDialog = false;
        bool m_DeleteFromDisk = false;

        // Selection & inputs
        std::uint64_t m_SelectedPrefabID = 0;       // use your AssetID if preferred
        std::uint64_t m_PrefabToDelete = 0;       // use your AssetID if preferred
        char          m_PrefabNameBuffer[256] = {};

        // Cached list for rendering (name, uid)
        std::vector<std::pair<std::string, std::uint64_t>> m_LoadedPrefabs;

        // External state (typical: currently selected entity in editor)
        entt::entity m_SelectedEntity = entt::null;
    };

} // namespace EditorUI
