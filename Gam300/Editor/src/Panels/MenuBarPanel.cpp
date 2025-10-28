#include "Panel/MenuBarPanel.h"

// Include only if you need the “create empty” path exactly like your snippet.
// Otherwise you can keep it UI-only and push this logic into a callback.
// #include "ECS/Entity.h"
// #include "Components/InfoComponent.h"
// #include "Components/TransformComponent.h"

#include <string>

// Engine headers used by this file (adjust to your project)
#include "Context/DebugHelpers.h"   // BOOM_INFO / BOOM_ERROR
// Application should provide NewScene/SaveScene/Stop/GetCurrentScenePath/IsSceneLoaded
// Context should provide renderer->IsDrawDebugMode() if you wire the Options menu

// ---------------------------------- Helpers ---------------------------------

void MenuBarPanel::PrefillSceneNameFromCurrent()
{
    if (!m.app || !m.sceneNameBuffer || m.sceneNameBufferSize == 0) return;
    if (!m.app->IsSceneLoaded()) return;

    std::string currentPath = m.app->GetCurrentScenePath();
    if (currentPath.empty()) return;

    const size_t lastSlash = currentPath.find_last_of("/\\");
    const size_t lastDot = currentPath.find_last_of(".");
    if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash) {
        const std::string sceneName = currentPath.substr(lastSlash + 1, lastDot - lastSlash - 1);

#ifdef _MSC_VER
        strncpy_s(m.sceneNameBuffer, m.sceneNameBufferSize, sceneName.c_str(), _TRUNCATE);
#else
        std::snprintf(m.sceneNameBuffer, m.sceneNameBufferSize, "%s", sceneName.c_str());
#endif
    }
}

// ----------------------------------- UI -------------------------------------

void MenuBarPanel::Render()
{
    if (!ImGui::BeginMainMenuBar())
        return;

    // --------------------------- File ----------------------------------------
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
            if (m.app) {
                m.app->NewScene("UntitledScene");
                if (m.RefreshSceneList) m.RefreshSceneList(true);
                BOOM_INFO("[Editor] Created new scene");
            }
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
            if (m.showSaveDialog) *m.showSaveDialog = true;
            if (m.app && m.app->IsSceneLoaded()) {
                if (m.RefreshSceneList) m.RefreshSceneList(true);
                PrefillSceneNameFromCurrent();
            }
        }

        if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) {
            if (m.showSaveDialog) *m.showSaveDialog = true;
            if (m.sceneNameBuffer && m.sceneNameBufferSize) {
                m.sceneNameBuffer[0] = '\0'; // clear for a fresh name
            }
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Load Scene", "Ctrl+O")) {
            if (m.showLoadDialog) *m.showLoadDialog = true;
            if (m.RefreshSceneList) m.RefreshSceneList(false);
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Exit", "Alt+F4")) {
            if (m.app) m.app->Stop();
        }

        ImGui::EndMenu();
    }

    // --------------------------- View ----------------------------------------
    if (ImGui::BeginMenu("View"))
    {
        if (m.showInspector)        ImGui::MenuItem("Inspector", nullptr, m.showInspector);
        if (m.showHierarchy)        ImGui::MenuItem("Hierarchy", nullptr, m.showHierarchy);
        if (m.showViewport)         ImGui::MenuItem("Viewport", nullptr, m.showViewport);
        if (m.showPrefabBrowser)    ImGui::MenuItem("Prefab Browser", nullptr, m.showPrefabBrowser);
        if (m.showPerformance)      ImGui::MenuItem("Performance", nullptr, m.showPerformance);
        if (m.showPlaybackControls) ImGui::MenuItem("Playback Controls", nullptr, m.showPlaybackControls);
        if (m.showConsole)          ImGui::MenuItem("Debug Console", nullptr, m.showConsole);
        if (m.showAudio)            ImGui::MenuItem("Audio", nullptr, m.showAudio);
        ImGui::EndMenu();
    }

    // --------------------------- Options -------------------------------------
    if (ImGui::BeginMenu("Options"))
    {
        // Toggle your renderer's debug draw flag by *reference* if available.
        // Ensure IsDrawDebugMode() returns a bool& or a pointer-like proxy.
        if (m.ctx && m.ctx->renderer) {
            ImGui::MenuItem("Debug Draw", nullptr, &m.ctx->renderer->IsDrawDebugMode());
        }
        ImGui::EndMenu();
    }

    // --------------------------- GameObjects ---------------------------------
    if (ImGui::BeginMenu("GameObjects"))
    {
        if (ImGui::MenuItem("Create Empty Object")) {
            // Leave the actual creation to your Editor/Context (keeps this panel UI-only).
            // If you want to do it here exactly like your snippet, move that code into a
            // lambda and call it from Editor when wiring MenuBarConfig.
            if (m.ctx) {
                // Example inline creation (uncomment/adapt if you prefer it here):
                // Entity e{ &m.ctx->scene };
                // e.Attach<InfoComponent>().name = "GameObject";
                // e.Attach<TransformComponent>();
                // if (m.selectedEntity) *m.selectedEntity = e.ID();
                if constexpr (true) {
                    BOOM_INFO("[Editor] Requested: Create Empty Object (delegate to Editor)");
                }
            }
        }

        if (ImGui::MenuItem("Create From Prefab...")) {
            if (m.showPrefabBrowser) *m.showPrefabBrowser = true;
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Save Selected as Prefab")) {
            if (m.selectedEntity && *m.selectedEntity != entt::null) {
                if (m.showSavePrefabDialog) *m.showSavePrefabDialog = true;
            }
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Delete Selected")) {
            // Deletion should be handled by your Editor/Context, since it touches the ECS.
            if (m.selectedEntity && *m.selectedEntity != entt::null) {
                BOOM_INFO("[Editor] Requested: Delete Selected (delegate to Editor)");
                // Example:
                // m.ctx->scene.destroy(*m.selectedEntity);
                // *m.selectedEntity = entt::null;
            }
        }

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}
