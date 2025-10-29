#pragma once

#include <functional>
#include <cstddef>
#include <memory>
#include <string>
#include "Context/Context.h"
#include "BoomEngine.h"

// ImGui
#include "Vendors/imgui/imgui.h"

// If you use entt in your editor state:
#include <entt/entity/entity.hpp>

// Forward decls of your engine types (adjust includes if you prefer)
struct Application;
struct Context; // whatever your context class is named (m_Context)

// A tiny bundle of pointers/callbacks so the panel stays UI-only.
struct MenuBarConfig {
    // Engine pointers
    Boom::Application* app{ nullptr };
    Context* ctx{ nullptr };

    // View toggles (wired to your editor state)
    bool* showInspector{ nullptr };
    bool* showHierarchy{ nullptr };
    bool* showViewport{ nullptr };
    bool* showPrefabBrowser{ nullptr };
    bool* showPerformance{ nullptr };
    bool* showPlaybackControls{ nullptr };
    bool* showConsole{ nullptr };
    bool* showAudio{ nullptr };

    // Dialog flags
    bool* showSaveDialog{ nullptr };
    bool* showLoadDialog{ nullptr };
    bool* showSavePrefabDialog{ nullptr };

    // Selected entity handle (optional; used by “Save/Delete Selected”)
    entt::entity* selectedEntity{ nullptr };

    // Scene name text buffer (for Save/Save As defaults)
    char* sceneNameBuffer{ nullptr };
    size_t sceneNameBufferSize{ 0 };

    // Helpers
    std::function<void(bool)> RefreshSceneList; // pass true to force
};

class MenuBarPanel {
public:
    MenuBarPanel() = default;
    explicit MenuBarPanel(const MenuBarConfig& cfg) { SetConfig(cfg); }

    void SetConfig(const MenuBarConfig& cfg) { m = cfg; }

    // Render the main menu bar
    void Render();

private:
    void PrefillSceneNameFromCurrent();

private:
    MenuBarConfig m{};
};

