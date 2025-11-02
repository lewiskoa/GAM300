#pragma once

#include <deque>
#include <string>
#include <array>

// Keep headers light in panel headers to avoid cycles.
// We only need IWidget (base) and the Entity type.
// If IWidget/Entity live in another header, include that one instead.
#include "Context/Widgets.h"    // provides IWidget and Entity (adjust if yours differs)

// Forward declare to avoid pulling imgui everywhere from the header.
struct ImGuiTextFilter;

namespace EditorUI
{
    // ImGui-based in-editor console
    struct ConsolePanel : IWidget
    {
        explicit ConsolePanel(AppInterface* c);

        void Clear();
        void AddLog(const char* fmt, ...) IM_FMTARGS(2);
        void TrackLastItemAsViewport(const char* label = "Viewport");

        // IWidget overrides
        void Render();
        void OnSelect(Entity entity) override;

        // Debug helpers
        void DebugConsoleState() const;

    private:
        std::deque<std::string> m_Lines;
        ImGuiTextFilter* m_FilterPtr = nullptr;   // we'll own a small filter object in the .cpp

        bool  m_Open = true;
        bool  m_AutoScroll = true;
        bool  m_Pause = false;
        int   m_MaxLines = 2000;

        bool   m_LogMouseMoves = true;
        bool   m_LogMouseClicks = true;
        float  m_LogEverySeconds = 0.05f;
        ImVec2 m_LastMouse{ -FLT_MAX, -FLT_MAX };
        double m_LastLogTime = 0.0;

        std::array<bool, ImGuiKey_NamedKey_END> m_KeyDownPrev{};
        char  m_InputBuf[256]{};
        bool  m_FocusInput = false;
    };
} // namespace EditorUI
