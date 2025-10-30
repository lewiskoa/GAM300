#include "Panels/ConsolePanel.h"

#include <cstdarg>   // va_list
#include <cstdio>    // vsnprintf
#include <cmath>     // std::sqrt
#include <algorithm> // std::min
#include <memory>

// ImGui (full definitions here)
#include "Vendors/imgui/imgui.h"

// Engine/Editor bits used in the implementation (safe to include here)
#include "Context/Context.h"
#include "Context/DebugHelpers.h"
#include "EditorPCH.h"

#ifndef ICON_FA_TERMINAL
#define ICON_FA_TERMINAL ""
#endif

namespace EditorUI
{
    // We keep the filter object internal to the .cpp so the header doesn't pull in imgui.h
    struct ConsoleFilterHolder
    {
        ImGuiTextFilter Filter;
    };

    // ---------------------------
    // ctor
    // ---------------------------
    ConsolePanel::ConsolePanel(AppInterface* c)
        : IWidget(c)
        , m_FilterPtr(new ImGuiTextFilter())
    {
        DEBUG_DLL_BOUNDARY("ConsolePanel::Constructor");
        DEBUG_POINTER(context, "AppInterface");

        if (!context) {
            BOOM_ERROR("ConsolePanel::Constructor - Null context!");
        }
        else {
            BOOM_INFO("ConsolePanel::Constructor - OK");
        }

        m_KeyDownPrev.fill(false);
        m_InputBuf[0] = '\0';
    }

    // ---------------------------
    // public logging API
    // ---------------------------
    void ConsolePanel::Clear()
    {
        m_Lines.clear();
    }

    void ConsolePanel::AddLog(const char* fmt, ...)
    {
        if (m_Pause) return;

        char buf[768];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        if ((int)m_Lines.size() >= m_MaxLines)
            m_Lines.pop_front();

        m_Lines.emplace_back(buf);
    }

    // ---------------------------------------------
    // Track the last ImGui item as a "viewport"
    // ---------------------------------------------
    void ConsolePanel::TrackLastItemAsViewport(const char* label)
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
                label ? label : "Viewport",
                mouseLocal.x, mouseLocal.y, mouseGlobal.x, mouseGlobal.y, size.x, size.y);
            m_LastMouse = mouseLocal;
            m_LastLogTime = now;
        }

        if (inside && m_LogMouseClicks) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                AddLog("[%s] Click: LMB @ local(%.1f, %.1f)", label ? label : "Viewport", mouseLocal.x, mouseLocal.y);
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                AddLog("[%s] Click: RMB @ local(%.1f, %.1f)", label ? label : "Viewport", mouseLocal.x, mouseLocal.y);
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
                AddLog("[%s] Click: MMB @ local(%.1f, %.1f)", label ? label : "Viewport", mouseLocal.x, mouseLocal.y);
        }
    }

    // ---------------------------
    // IWidget overrides
    // ---------------------------
    void ConsolePanel::Render()
    {
        if (!context) {
            BOOM_ERROR("ConsolePanel::OnShow - Null context!");
            return;
        }

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

        // B) Log text characters received this frame
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

            // Use the filter object
            ImGuiTextFilter& filter = *static_cast<ImGuiTextFilter*>(m_FilterPtr);
            filter.Draw("Filter");

            ImGui::Separator();

            // Scroll area (reserve space for one line of input at bottom)
            const float inputRowHeight = ImGui::GetFrameHeightWithSpacing() + 4.0f;
            ImGui::BeginChild("ConsoleScroll", ImVec2(0, -inputRowHeight), false, ImGuiWindowFlags_HorizontalScrollbar);
            for (const auto& s : m_Lines) {
                if (!filter.PassFilter(s.c_str())) continue;
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

    void ConsolePanel::OnSelect(Entity entity)
    {
        DEBUG_DLL_BOUNDARY("ConsolePanel::OnSelect");
        BOOM_INFO("ConsolePanel::OnSelect - Entity selected: {}", (uint32_t)entity);
    }

    void ConsolePanel::DebugConsoleState() const
    {
        BOOM_INFO("=== ConsolePanel Debug State ===");
        BOOM_INFO("Lines: {}", (int)m_Lines.size());
        BOOM_INFO("MaxLines: {}", m_MaxLines);
        BOOM_INFO("AutoScroll: {}", m_AutoScroll);
        BOOM_INFO("Pause: {}", m_Pause);
        BOOM_INFO("LogMouseMoves: {}", m_LogMouseMoves);
        BOOM_INFO("LogMouseClicks: {}", m_LogMouseClicks);
        BOOM_INFO("=== End Debug State ===");
    }
} // namespace EditorUI
