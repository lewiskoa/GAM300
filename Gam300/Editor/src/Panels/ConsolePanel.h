#pragma once
#include "Core.h"
#include "Context/Context.h"
#include "Context/DebugHelpers.h"
#include "Vendors/imgui/imgui.h"

#ifndef ICON_FA_TERMINAL
#define ICON_FA_TERMINAL ""
#endif

// ConsolePanel provides an ImGui-based in-editor console.
struct ConsolePanel : IWidget
{
    BOOM_INLINE ConsolePanel(AppInterface* c);
    BOOM_INLINE void Clear();
    BOOM_INLINE void AddLog(const char* fmt, ...) IM_FMTARGS(2);
    BOOM_INLINE void TrackLastItemAsViewport(const char* label = "Viewport");
    BOOM_INLINE void OnShow() override;
    BOOM_INLINE void OnSelect(Entity entity) override;
    BOOM_INLINE void DebugConsoleState() const;

private:
    std::deque<std::string> m_Lines;
    ImGuiTextFilter         m_Filter;

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
    char m_InputBuf[256]{};
    bool m_FocusInput = false;
};
