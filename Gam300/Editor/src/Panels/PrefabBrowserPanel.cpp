#include "Panels/PrefabBrowserPanel.h"

// Pull full types here (keeps header light)
#include "Editor/Editor.h"
#include "Context/Context.h"
#include "Context/DebugHelpers.h"

// Assets / prefabs API (adjust to your project)
#include "Auxiliaries/Assets.h"          // AssetID, Assets, PrefabAsset, EMPTY_ASSET
#include "Auxiliaries/PrefabUtility.h"   // CreatePrefabFromEntity, SavePrefab, LoadPrefab, Instantiate

#include <algorithm>
#include <cctype>
#include <cstring> // std::strlen, std::snprintf

namespace EditorUI {

    PrefabBrowserPanel::PrefabBrowserPanel(Editor* owner)
        : m_Owner(owner)
    {
        DEBUG_DLL_BOUNDARY("PrefabBrowserPanel::Constructor");

        if (!m_Owner) {
            BOOM_ERROR("PrefabBrowserPanel::Constructor - Null owner!");
            return;
        }

        // Editor must expose: Boom::AppContext* GetContext() const;
        m_Ctx = m_Owner->GetContext();
        DEBUG_POINTER(m_Ctx, "AppContext");

        if (!m_Ctx) {
            BOOM_ERROR("PrefabBrowserPanel::Constructor - Null AppContext!");
            return;
        }

        m_PrefabNameBuffer[0] = '\0';
        RefreshPrefabList();
    }

    void PrefabBrowserPanel::Render()
    {
        OnShow();
    }

    void PrefabBrowserPanel::OnShow()
    {
        if (!m_ShowPrefabBrowser) return;

        // 1) Modal dialogs first — they're opened by flags
        RenderPrefabDialogs();

        // 2) Then the main browser window
        RenderPrefabBrowser();
    }

    // -----------------------------------------------------------------------------
    // UI: Dialogs (Save / Delete)
    // -----------------------------------------------------------------------------
    void PrefabBrowserPanel::RenderPrefabDialogs()
    {
        if (!m_Ctx || !m_Ctx->assets) return;

        // --- Save Prefab Dialog ---------------------------------------------------
        if (m_ShowSavePrefabDialog) {
            ImGui::OpenPopup("Save as Prefab");
            m_ShowSavePrefabDialog = false;
        }

        if (ImGui::BeginPopupModal("Save as Prefab", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Save selected entity as prefab:");
            ImGui::Separator();

            bool enterPressed = ImGui::InputText(
                "Prefab Name",
                m_PrefabNameBuffer,
                sizeof(m_PrefabNameBuffer),
                ImGuiInputTextFlags_EnterReturnsTrue
            );

            ImGui::Separator();

            bool saveClicked = ImGui::Button("Save", ImVec2(80, 0));
            ImGui::SameLine();
            bool cancelClicked = ImGui::Button("Cancel", ImVec2(80, 0));

            if ((saveClicked || enterPressed) && std::strlen(m_PrefabNameBuffer) > 0) {
                const AssetID prefabID = RandomU64();
                auto prefab = PrefabUtility::CreatePrefabFromEntity(
                    *m_Ctx->assets,
                    prefabID,
                    std::string(m_PrefabNameBuffer),
                    m_Ctx->scene,
                    m_SelectedEntity
                );

                if (prefab) {
                    std::string filepath = "Prefabs/" + std::string(m_PrefabNameBuffer) + ".prefab";
                    if (PrefabUtility::SavePrefab(*prefab, filepath)) {
                        BOOM_INFO("[Editor] Saved prefab '{}'", m_PrefabNameBuffer);
                        RefreshPrefabList();
                    }
                }
                ImGui::CloseCurrentPopup();
            }

            if (cancelClicked) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        // --- Delete Prefab Dialog -------------------------------------------------
        if (m_ShowDeletePrefabDialog) {
            ImGui::OpenPopup("Delete Prefab?");
            m_ShowDeletePrefabDialog = false;
            m_DeleteFromDisk = false; // Reset checkbox
        }

        if (ImGui::BeginPopupModal("Delete Prefab?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            auto& asset = m_Ctx->assets->Get<PrefabAsset>(m_PrefabToDelete);
            ImGui::Text("Delete prefab '%s'?", asset.name.c_str());
            ImGui::Spacing();

            ImGui::Checkbox("Delete from disk", &m_DeleteFromDisk);
            if (m_DeleteFromDisk) {
                ImGui::TextColored(ImVec4(1, 0.3f, 0, 1), "Warning: This cannot be undone!");
            }

            ImGui::Separator();

            if (ImGui::Button("Delete", ImVec2(120, 0))) {
                // Save info before erasing
                const std::string filepath = "Prefabs/" + asset.name + ".prefab";
                const std::string name = asset.name;

                // Remove from memory FIRST
                m_Ctx->assets->GetMap<PrefabAsset>().erase(m_PrefabToDelete);

                // NOW delete from disk (using the saved filepath)
                if (m_DeleteFromDisk) {
                    if (std::filesystem::exists(filepath)) {
                        std::filesystem::remove(filepath);
                        BOOM_INFO("[Editor] Deleted prefab file: {}", filepath);
                    }
                    else {
                        BOOM_WARN("[Editor] Prefab file not found: {}", filepath);
                    }
                }

                BOOM_INFO("[Editor] Deleted prefab '{}' from memory", name);
                RefreshPrefabList();

                if (m_SelectedPrefabID == m_PrefabToDelete) {
                    m_SelectedPrefabID = 0; // EMPTY
                }
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    // -----------------------------------------------------------------------------
    // UI: Main Prefab Browser window
    // -----------------------------------------------------------------------------
    void PrefabBrowserPanel::RenderPrefabBrowser()
    {
        if (!m_Ctx || !m_Ctx->assets) return;

        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Prefab Browser", &m_ShowPrefabBrowser)) {

            // Toolbar
            if (ImGui::Button("Refresh", ImVec2(80, 0))) {
                RefreshPrefabList();
                LoadAllPrefabsFromDisk(); // optional: ensure newest files are imported
            }

            ImGui::SameLine();
            ImGui::Text("Prefabs: %d", (int)m_LoadedPrefabs.size());
            ImGui::Separator();

            // Search bar
            static char searchBuffer[256] = "";
            ImGui::SetNextItemWidth(-1);
            ImGui::InputTextWithHint("##Search", "Search prefabs...", searchBuffer, sizeof(searchBuffer));
            ImGui::Separator();

            // Prefab list
            ImGui::BeginChild("PrefabList", ImVec2(0, -40), true);

            auto& prefabMap = m_Ctx->assets->GetMap<PrefabAsset>();

            std::string search = searchBuffer;
            std::transform(search.begin(), search.end(), search.begin(),
                [](unsigned char c) { return (char)std::tolower(c); });

            int count = 0;
            for (auto& [uid, assetPtr] : prefabMap) {
                if (uid == EMPTY_ASSET) continue;
                auto* asset = static_cast<PrefabAsset*>(assetPtr.get());
                if (!asset) continue;

                // Filter by search
                std::string name = asset->name;
                std::string nameLower = name;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(),
                    [](unsigned char c) { return (char)std::tolower(c); });

                if (!search.empty() && nameLower.find(search) == std::string::npos) {
                    continue;
                }

                count++;
                ImGui::PushID((int)uid);

                bool selected = (m_SelectedPrefabID == uid);
                if (ImGui::Selectable(("## " + name).c_str(), selected, 0, ImVec2(0, 40))) {
                    m_SelectedPrefabID = uid;
                }

                // Double-click to instantiate
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                    entt::entity newEntity = PrefabUtility::Instantiate(m_Ctx->scene, *m_Ctx->assets, uid);
                    if (newEntity != entt::null) {
                        m_SelectedEntity = newEntity;
                        BOOM_INFO("[Editor] Instantiated prefab '{}'", name);
                    }
                }

                // Context menu
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Instantiate")) {
                        entt::entity newEntity = PrefabUtility::Instantiate(m_Ctx->scene, *m_Ctx->assets, uid);
                        if (newEntity != entt::null) {
                            m_SelectedEntity = newEntity;
                        }
                    }
                    if (ImGui::MenuItem("Save to Disk")) {
                        std::string path = "Prefabs/" + name + ".prefab";
                        PrefabUtility::SavePrefab(*asset, path);
                        BOOM_INFO("[Editor] Saved prefab '{}'", name);
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Delete", nullptr, false, true)) {
                        m_PrefabToDelete = uid;
                        m_ShowDeletePrefabDialog = true;
                    }
                    ImGui::EndPopup();
                }

                // Simple icon + meta on the same row
                ImVec2 p = ImGui::GetItemRectMin();
                ImDrawList* draw = ImGui::GetWindowDrawList();
                ImVec2 iconMin = ImVec2(p.x + 5, p.y + 5);
                ImVec2 iconMax = ImVec2(p.x + 35, p.y + 35);
                draw->AddRectFilled(iconMin, iconMax, IM_COL32(80, 120, 180, 255), 4.0f);
                draw->AddText(ImVec2(iconMin.x + 8, iconMin.y + 8), IM_COL32(255, 255, 255, 255), "P");

                draw->AddText(ImVec2(p.x + 45, p.y + 5), IM_COL32(255, 255, 255, 255), name.c_str());

                char metaText[128];
                std::snprintf(metaText, sizeof(metaText), "ID: ...%llu",
                    (unsigned long long)(uid % 100000));
                draw->AddText(ImVec2(p.x + 45, p.y + 22), IM_COL32(150, 150, 150, 255), metaText);

                ImGui::PopID();
            }

            if (count == 0) {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);
                ImGui::TextDisabled("No prefabs found");
                ImGui::TextDisabled("Create one via: GameObject > Save Selected as Prefab");
            }

            ImGui::EndChild();

            // Bottom toolbar
            ImGui::Separator();
            if (m_SelectedPrefabID != 0) {
                auto& asset = m_Ctx->assets->Get<PrefabAsset>(m_SelectedPrefabID);
                ImGui::Text("Selected: %s", asset.name.c_str());
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - 100);
                if (ImGui::Button("Instantiate", ImVec2(100, 0))) {
                    entt::entity newEntity = PrefabUtility::Instantiate(m_Ctx->scene, *m_Ctx->assets, m_SelectedPrefabID);
                    if (newEntity != entt::null) {
                        m_SelectedEntity = newEntity;
                        BOOM_INFO("[Editor] Instantiated prefab '{}'", asset.name);
                    }
                }
            }
            else {
                ImGui::TextDisabled("No prefab selected");
            }
        }
        ImGui::End();
    }

    // -----------------------------------------------------------------------------
    // Data ops
    // -----------------------------------------------------------------------------
    void PrefabBrowserPanel::RefreshPrefabList()
    {
        m_LoadedPrefabs.clear();
        if (!m_Ctx || !m_Ctx->assets) return;

        auto& map = m_Ctx->assets->GetMap<PrefabAsset>();
        m_LoadedPrefabs.reserve(map.size());
        for (auto& [uid, assetPtr] : map) {
            if (uid == EMPTY_ASSET) continue;
            auto* asset = static_cast<PrefabAsset*>(assetPtr.get());
            if (!asset) continue;
            m_LoadedPrefabs.emplace_back(asset->name, uid);
        }
    }

    void PrefabBrowserPanel::LoadAllPrefabsFromDisk()
    {
        if (!m_Ctx || !m_Ctx->assets) return;

        const std::filesystem::path kDir{ "Prefabs" };
        if (!std::filesystem::exists(kDir)) return;

        for (auto& e : std::filesystem::directory_iterator(kDir)) {
            if (!e.is_regular_file()) continue;
            const auto& p = e.path();
            if (p.extension() == ".prefab") {
                try {
                    PrefabUtility::LoadPrefab(*m_Ctx->assets, p.string());
                }
                catch (const std::exception& ex) {
                    BOOM_ERROR("[Editor] Failed to load prefab '{}': {}", p.string(), ex.what());
                }
            }
        }

        RefreshPrefabList();
    }

} // namespace EditorUI
