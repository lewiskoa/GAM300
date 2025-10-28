#pragma once
#include "Core.h"
#include "Context/Context.h"
#include "Context/DebugHelpers.h"
#include "Vendors/imgui/imgui.h"

#ifndef ICON_FA_TERMINAL
#define ICON_FA_TERMINAL ""
#endif

// A UI-only debug console that renders inside ImGui (no stdout prints).
// It also exposes TrackLastItemAsViewport(...) so your ViewportWindow
// can log mouse local coords & clicks for the last ImGui item (the Image).
struct ConsoleWindow : IWidget
{
    BOOM_INLINE ConsoleWindow(AppInterface* c) : IWidget(c)
    {
        DEBUG_DLL_BOUNDARY("ConsoleWindow::Constructor");
        DEBUG_POINTER(context, "AppInterface");

        if (!context) {
            BOOM_ERROR("ConsoleWindow::Constructor - Null context!");
        }
        else {
            BOOM_INFO("ConsoleWindow::Constructor - OK");
        }

        m_KeyDownPrev.fill(false);
        m_InputBuf[0] = '\0';
    }

    // ---------------------------
    // Public logging API
    // ---------------------------
    BOOM_INLINE void Clear() { m_Lines.clear(); }

    BOOM_INLINE void AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        if (m_Pause) return;
        char buf[768];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        if ((int)m_Lines.size() >= m_MaxLines) m_Lines.pop_front();
        m_Lines.emplace_back(buf);
    }

    // Call this RIGHT AFTER drawing your viewport Image item in ViewportWindow.
    // Example use:
    //   ImGui::Image(...);
    //   console.TrackLastItemAsViewport("SceneViewport");
    //
    // Logs mouse local coords (relative to that Image) + click events,
    // with throttling to avoid spam.
    BOOM_INLINE void TrackLastItemAsViewport(const char* label = "Viewport")
    {
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();
        ImVec2 size{ max.x - min.x, max.y - min.y };

        const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup |
            ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

        ImVec2 mouseGlobal = ImGui::GetMousePos();
        ImVec2 mouseLocal{ mouseGlobal.x - min.x, mouseGlobal.y - min.y };

        const bool inside = hovered &&
            mouseLocal.x >= 0 && mouseLocal.y >= 0 &&
            mouseLocal.x <= size.x && mouseLocal.y <= size.y;

        double now = ImGui::GetTime();
        float dx = mouseLocal.x - m_LastMouse.x;
        float dy = mouseLocal.y - m_LastMouse.y;
        float deltaDist = (m_LastMouse.x == -FLT_MAX) ? 1e9f : std::sqrt(dx * dx + dy * dy);

        if (inside && m_LogMouseMoves && (now - m_LastLogTime) >= m_LogEverySeconds && deltaDist >= 0.5f) {
            AddLog("[%s] Mouse local(%.1f, %.1f)  global(%.1f, %.1f)  size(%.0f x %.0f)",
                label, mouseLocal.x, mouseLocal.y, mouseGlobal.x, mouseGlobal.y, size.x, size.y);
            m_LastMouse = mouseLocal;
            m_LastLogTime = now;
        }

        if (inside && m_LogMouseClicks) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                AddLog("[%s] Click: LMB @ local(%.1f, %.1f)", label, mouseLocal.x, mouseLocal.y);
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                AddLog("[%s] Click: RMB @ local(%.1f, %.1f)", label, mouseLocal.x, mouseLocal.y);
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
                AddLog("[%s] Click: MMB @ local(%.1f, %.1f)", label, mouseLocal.x, mouseLocal.y);
        }
    }

    // ---------------------------
    // IWidget overrides
    // ---------------------------
    BOOM_INLINE void OnShow() override
    {
        //DEBUG_DLL_BOUNDARY("ConsoleWindow::OnShow");

        if (!context) {
            BOOM_ERROR("ConsoleWindow::OnShow - Null context!");
            return;
        }

        // --- Live keyboard capture (runs regardless of window visibility) ---
        ImGuiIO& io = ImGui::GetIO();

        // A) Log new key-press transitions (no repeats)
        for (int k = (int)ImGuiKey_NamedKey_BEGIN; k < (int)ImGuiKey_NamedKey_END; ++k) {
            ImGuiKey key = (ImGuiKey)k;
            bool down = ImGui::IsKeyDown(key);
            if (down && !m_KeyDownPrev[k]) {
                const char* name = ImGui::GetKeyName(key);
                AddLog("[KeyDown] %s", (name && *name) ? name : "(Unknown)");
            }
            m_KeyDownPrev[k] = down;
        }

        // B) Log text characters that ImGui received this frame
        if (!io.InputQueueCharacters.empty()) {
            for (ImWchar c : io.InputQueueCharacters) {
                if (c >= 0x20 && c != 0x7F)
                    AddLog("[Char] '%c' (U+%04X)", (char)c, (unsigned)c);
                else
                    AddLog("[Char] U+%04X", (unsigned)c);
            }
        }

        if (ImGui::Begin(ICON_FA_TERMINAL "\tDebug Console", &m_Open))
        {
            // Toolbar
            if (ImGui::Button("Clear")) Clear();
            ImGui::SameLine();
            ImGui::Checkbox("Auto-scroll", &m_AutoScroll);
            ImGui::SameLine();
            ImGui::Checkbox("Pause", &m_Pause);
            ImGui::SameLine();
            ImGui::Checkbox("Log mouse moves", &m_LogMouseMoves);
            ImGui::SameLine();
            ImGui::Checkbox("Log clicks", &m_LogMouseClicks);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(180.f);
            m_Filter.Draw("Filter");

            ImGui::Separator();

            // Scroll area (reserve space for one line of input at bottom)
            const float inputRowHeight = ImGui::GetFrameHeightWithSpacing() + 4.0f;
            ImGui::BeginChild("ConsoleScroll", ImVec2(0, -inputRowHeight), false, ImGuiWindowFlags_HorizontalScrollbar);
            for (const auto& s : m_Lines) {
                if (!m_Filter.PassFilter(s.c_str())) continue;
                ImGui::TextUnformatted(s.c_str());
            }
            if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();

            // --- Command input line ---
            ImGui::Separator();
            ImGui::SetNextItemWidth(-1.0f);
            ImGuiInputTextFlags itf = ImGuiInputTextFlags_EnterReturnsTrue;
            if (m_FocusInput) { ImGui::SetKeyboardFocusHere(); m_FocusInput = false; }
            if (ImGui::InputText("##ConsoleInput", m_InputBuf, IM_ARRAYSIZE(m_InputBuf), itf)) {
                if (m_InputBuf[0]) {
                    AddLog("> %s", m_InputBuf);    // echo the command
                    // TODO: parse/execute commands here if desired
                    m_InputBuf[0] = '\0';
                }
                m_FocusInput = true; // keep focus after submit
            }
        }
        ImGui::End();
    }

    BOOM_INLINE void OnSelect(Entity entity) override
    {
        DEBUG_DLL_BOUNDARY("ConsoleWindow::OnSelect");
        BOOM_INFO("ConsoleWindow::OnSelect - Entity selected: {}", (uint32_t)entity);
    }

    BOOM_INLINE void DebugConsoleState() const
    {
        BOOM_INFO("=== ConsoleWindow Debug State ===");
        BOOM_INFO("Lines: {}", (int)m_Lines.size());
        BOOM_INFO("MaxLines: {}", m_MaxLines);
        BOOM_INFO("AutoScroll: {}", m_AutoScroll);
        BOOM_INFO("Pause: {}", m_Pause);
        BOOM_INFO("LogMouseMoves: {}", m_LogMouseMoves);
        BOOM_INFO("LogMouseClicks: {}", m_LogMouseClicks);
        BOOM_INFO("=== End Debug State ===");
    }

private:
    // Data
    std::deque<std::string> m_Lines;
    ImGuiTextFilter         m_Filter;

    bool  m_Open = true;
    bool  m_AutoScroll = true;
    bool  m_Pause = false;
    int   m_MaxLines = 2000;

    // Mouse tracking config/state
    bool   m_LogMouseMoves = true;
    bool   m_LogMouseClicks = true;
    float  m_LogEverySeconds = 0.05f; // throttle mouse move logs
    ImVec2 m_LastMouse{ -FLT_MAX, -FLT_MAX };
    double m_LastLogTime = 0.0;

    // Keyboard tracking (named key space)
    std::array<bool, ImGuiKey_NamedKey_END> m_KeyDownPrev{};

    // Command input line
    char m_InputBuf[256]{};
    bool m_FocusInput = false;
};
