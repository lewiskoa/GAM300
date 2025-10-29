#include "Panels/PrefabBrowserPanel.h"

// Adjust these includes to your project structure:
#include "Auxiliaries/Assets.h"       // Assets storage (Assets*, GetMap<PrefabAsset>(), etc.)

#include <algorithm>
#include <cstring>  // for std::strlen / std::strcpy
#include "Editor/EditorPCH.h"

namespace EditorUI
{
    PrefabBrowserPanel::PrefabBrowserPanel(AppInterface* ctx)
        : IWidget(ctx)
    {
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

    // -------------------------------------------------------------------------
    // UI: Dialogs (Save / Delete)
    // -------------------------------------------------------------------------
    void PrefabBrowserPanel::RenderPrefabDialogs()
    {
        auto* appCtx = GetAppContext(); // ✅ Get typed context once

        // --- Save Prefab Dialog ------------------------------------------------
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
                AssetID prefabID = RandomU64();
                auto prefab = PrefabUtility::CreatePrefabFromEntity(
                    *appCtx->assets,  // ✅ Use appCtx
                    prefabID,
                    std::string(m_PrefabNameBuffer),
                    appCtx->scene,    // ✅ Use appCtx
                    m_SelectedEntity
                );

                if (prefab) {
                    std::string filepath = "Prefabs/" + std::string(m_PrefabNameBuffer) + ".prefab";
                    bool saved = PrefabUtility::SavePrefab(*prefab, filepath);
                    if (saved) {
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

        // --- Delete Prefab Dialog ---------------------------------------------
        if (m_ShowDeletePrefabDialog) {
            ImGui::OpenPopup("Delete Prefab?");
            m_ShowDeletePrefabDialog = false;
            m_DeleteFromDisk = false; // Reset checkbox
        }

        if (ImGui::BeginPopupModal("Delete Prefab?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            auto& asset = appCtx->assets->Get<PrefabAsset>(m_PrefabToDelete);  // ✅ Use appCtx
            ImGui::Text("Delete prefab '%s'?", asset.name.c_str());
            ImGui::Spacing();

            ImGui::Checkbox("Delete from disk", &m_DeleteFromDisk);
            if (m_DeleteFromDisk) {
                ImGui::TextColored(ImVec4(1, 0.3f, 0, 1), "Warning: This cannot be undone!");
            }

            ImGui::Separator();

            if (ImGui::Button("Delete", ImVec2(120, 0))) {
                // SAVE the filepath BEFORE erasing the asset
                std::string filepath = "Prefabs/" + asset.name + ".prefab";
                std::string name = asset.name;

                // Remove from memory FIRST
                appCtx->assets->GetMap<PrefabAsset>().erase(m_PrefabToDelete);  // ✅ Use appCtx

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
                    m_SelectedPrefabID = EMPTY_ASSET;
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

    // -------------------------------------------------------------------------
    // UI: Main Prefab Browser window
    // -------------------------------------------------------------------------
    void PrefabBrowserPanel::RenderPrefabBrowser()
    {
        auto* appCtx = GetAppContext(); // ✅ Get typed context once

        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Prefab Browser", &m_ShowPrefabBrowser)) {

            // Toolbar
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));
            if (ImGui::Button("Refresh", ImVec2(80, 0))) {
                RefreshPrefabList();
                LoadAllPrefabsFromDisk(); // optional: ensure newest files are imported
            }
            ImGui::PopStyleColor();

            ImGui::SameLine();
            ImGui::TextDisabled("|");
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

            auto& prefabMap = appCtx->assets->GetMap<PrefabAsset>();  // ✅ Use appCtx

            std::string search = searchBuffer;
            std::transform(search.begin(), search.end(), search.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            int count = 0;
            for (auto& [uid, asset] : prefabMap) {
                if (uid == EMPTY_ASSET) continue;

                // Filter by search
                std::string name = asset->name;
                std::string nameLower = name;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                if (!search.empty() && nameLower.find(search) == std::string::npos) {
                    continue;
                }

                count++;
                ImGui::PushID((int)uid);

                // Selectable line
                bool selected = (m_SelectedPrefabID == uid);
                if (ImGui::Selectable(("## " + name).c_str(), selected, 0, ImVec2(0, 40))) {
                    m_SelectedPrefabID = uid;
                }

                // Double-click to instantiate
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                    EntityID newEntity = PrefabUtility::Instantiate(appCtx->scene, *appCtx->assets, uid);  // ✅ Use appCtx
                    if (newEntity != entt::null) {
                        m_SelectedEntity = newEntity;
                        BOOM_INFO("[Editor] Instantiated prefab '{}'", name);
                    }
                }

                // Right-click menu
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Instantiate")) {
                        EntityID newEntity = PrefabUtility::Instantiate(appCtx->scene, *appCtx->assets, uid);  // ✅ Use appCtx
                        if (newEntity != entt::null) {
                            m_SelectedEntity = newEntity;
                        }
                    }
                    if (ImGui::MenuItem("Save to Disk")) {
                        std::string path = "Prefabs/" + name + ".prefab";
                        PrefabUtility::SavePrefab(*static_cast<PrefabAsset*>(asset.get()), path);
                        BOOM_INFO("[Editor] Saved prefab '{}'", name);
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Delete", nullptr, false, true)) {
                        m_PrefabToDelete = uid;
                        m_ShowDeletePrefabDialog = true;
                    }
                    ImGui::EndPopup();
                }

                // Custom draw on the same line as the selectable
                ImVec2 p = ImGui::GetItemRectMin();
                ImDrawList* draw = ImGui::GetWindowDrawList();

                // Icon placeholder
                ImVec2 iconMin = ImVec2(p.x + 5, p.y + 5);
                ImVec2 iconMax = ImVec2(p.x + 35, p.y + 35);
                draw->AddRectFilled(iconMin, iconMax, IM_COL32(80, 120, 180, 255), 4.0f);
                draw->AddText(ImVec2(iconMin.x + 8, iconMin.y + 8), IM_COL32(255, 255, 255, 255), "P");

                // Name & meta
                draw->AddText(ImVec2(p.x + 45, p.y + 5), IM_COL32(255, 255, 255, 255), name.c_str());

                char metaText[128];
                std::snprintf(metaText, sizeof(metaText), "ID: ...%llu", (unsigned long long)(uid % 100000));
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
            if (m_SelectedPrefabID != EMPTY_ASSET) {
                auto& asset = appCtx->assets->Get<PrefabAsset>(m_SelectedPrefabID);  // ✅ Use appCtx
                ImGui::Text("Selected: %s", asset.name.c_str());
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - 100);
                if (ImGui::Button("Instantiate", ImVec2(100, 0))) {
                    EntityID newEntity = PrefabUtility::Instantiate(appCtx->scene, *appCtx->assets, m_SelectedPrefabID);  // ✅ Use appCtx
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



    void PrefabBrowserPanel::LoadAllPrefabsFromDisk()
    {
        auto* appCtx = GetAppContext(); // ✅ Get typed context

        // Optional: if you already have a project-level function that does this,
        // call it here instead. This implementation simply scans /Prefabs.
        const std::filesystem::path kDir{ "Prefabs" };
        if (!std::filesystem::exists(kDir)) return;

        for (auto& e : std::filesystem::directory_iterator(kDir)) {
            if (!e.is_regular_file()) continue;
            const auto& p = e.path();
            if (p.extension() == ".prefab") {
                try {
                    // Adjust if your API differs (e.g., PrefabUtility::LoadPrefab returns AssetID)
                    PrefabUtility::LoadPrefab(*appCtx->assets, p.string());  // ✅ Use appCtx
                }
                catch (std::exception const& ex) {
                    BOOM_ERROR("[Editor] Failed to load prefab '{}': {}", p.string(), ex.what());
                }
            }
        }

        RefreshPrefabList();
    }
} // namespace EditorUI