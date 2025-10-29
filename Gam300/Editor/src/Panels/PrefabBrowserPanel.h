#pragma once
#include "Context/Context.h"
#include "Context/DebugHelpers.h"
#include "Vendors/imgui/imgui.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "Editor/EditorPCH.h"



namespace EditorUI
{
    class PrefabBrowserPanel : public IWidget
    {
    public:
        BOOM_INLINE explicit PrefabBrowserPanel(AppInterface* ctx);

        // Call once per frame from your editor render pass
        void Render();              // wrapper -> calls OnShow()
        BOOM_INLINE void OnShow() override;

        // External toggles (optional)
        BOOM_INLINE void Show(bool v) { m_ShowPrefabBrowser = v; }
        BOOM_INLINE bool IsVisible() const { return m_ShowPrefabBrowser; }

        // To trigger the save/delete dialogs from outside (optional)
        BOOM_INLINE void OpenSaveDialog() { m_ShowSavePrefabDialog = true; }
        BOOM_INLINE void OpenDeleteDialog(AssetID id) { m_PrefabToDelete = id; m_ShowDeletePrefabDialog = true; }

    private:
        // UI sections
        void RenderPrefabDialogs();
        void RenderPrefabBrowser();

        // Data updates
        void RefreshPrefabList();
        void LoadAllPrefabsFromDisk(); // scans Prefabs/ and loads .prefab assets

        // Helper to get AppContext
        BOOM_INLINE Boom::AppContext* GetAppContext() {
            return static_cast<Boom::AppContext*>(m_Context);
        }

    private:
        // Visibility
        bool m_ShowPrefabBrowser = true;

        // Dialog states
        bool m_ShowSavePrefabDialog = false;
        bool m_ShowDeletePrefabDialog = false;
        bool m_DeleteFromDisk = false;

        // Selection & inputs
        AssetID m_SelectedPrefabID = EMPTY_ASSET;
        AssetID m_PrefabToDelete = EMPTY_ASSET;
        char    m_PrefabNameBuffer[256] = {};

        // Cached list for rendering (name, uid)
        std::vector<std::pair<std::string, AssetID>> m_LoadedPrefabs;

        // External state (typical editor members you referenced)
        EntityID m_SelectedEntity = entt::null;
    };
} // namespace EditorUI