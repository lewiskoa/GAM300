#pragma once
#include "Context/Context.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include "ImGuizmo.h"
#include "Context/Profiler.hpp"
#include "AppWindow.h"

namespace Boom { class Application; }

namespace EditorUI
{
    // Audio panel matching the previous EditorUI::Audio::Render() behavior
    class AudioPanel : public IWidget
    {
    public:
        BOOM_INLINE explicit AudioPanel(AppInterface* ctx);

        // Call this every frame from your editor render pass
        void Render();              // wrapper -> calls OnShow()
        BOOM_INLINE void OnShow() override;

        // Optional visibility control
        BOOM_INLINE void Show(bool v) { m_Show = v; }
        BOOM_INLINE bool IsVisible() const { return m_Show; }

    private:
        struct Track { std::string name; std::string path; };
        void EnsureVolumeKeys();

    private:
        bool m_Show = true;

        // UI state
        int  m_Selected = 0;
        bool m_Loop = false;
        bool m_Paused = false;

        // Catalog (same as your snippet; adjust paths as needed)
        std::vector<Track> m_Tracks;

        // Per-track volume
        std::unordered_map<std::string, float> m_Volume;
    };
} // namespace EditorUI
