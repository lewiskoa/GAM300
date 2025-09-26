// Windows/Console.h
#pragma once
#include <deque>
#include <mutex>
#include <unordered_map>
#include <functional>
#include "Context/Context.h"
#include "Context/DebugHelpers.h"
#include "Vendors/imgui/imgui.h"

struct ConsoleWindow : IWidget
{
    BOOM_INLINE ConsoleWindow(AppInterface* ctx) : IWidget(ctx)
    {
        DEBUG_DLL_BOUNDARY("ConsoleWindow::Constructor");
        DEBUG_POINTER(ctx, "AppInterface");

        // default built-ins
        Register("help", [this](const std::string&) {
            Log("Commands:");
            for (auto& [k, _] : m_Cmds) Log("  " + k);
            Log("  clear");
            });
        Register("clear", [this](const std::string&) {
            std::scoped_lock lk(m_Mu);
            m_Lines.clear();
            });

        BOOM_INFO("ConsoleWindow::Constructor - Ready");
    }

    BOOM_INLINE void OnShow(AppInterface* /*ctx*/) override
    {
        DEBUG_DLL_BOUNDARY("ConsoleWindow::OnShow");

        if (!ImGui::Begin(ICON_FA_TERMINAL "\tConsole", &open))
        {
            ImGui::End(); return;
        }

        // Toolbar
        if (ImGui::Button("Clear")) { std::scoped_lock lk(m_Mu); m_Lines.clear(); }
        ImGui::SameLine(); m_Filter.Draw("Filter", 180.0f);
        ImGui::SameLine(); ImGui::Checkbox("Auto-scroll", &m_AutoScroll);
        ImGui::Separator();

        // Log region
        ImGui::BeginChild("LogRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false,
            ImGuiWindowFlags_HorizontalScrollbar);
        {
            std::scoped_lock lk(m_Mu);
            for (auto& s : m_Lines)
                if (m_Filter.PassFilter(s.c_str()))
                    ImGui::TextUnformatted(s.c_str());

            if (m_RequestScroll || (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                ImGui::SetScrollHereY(1.0f);
            m_RequestScroll = false;
        }
        ImGui::EndChild();

        // Input line
        bool reclaim = false;
        ImGui::PushItemWidth(-1);
        if (ImGui::InputText("##ConsoleInput", m_Input, IM_ARRAYSIZE(m_Input),
            ImGuiInputTextFlags_EnterReturnsTrue |
            ImGuiInputTextFlags_CallbackHistory,
            &HistoryCB, this))
        {
            Execute(m_Input);
            m_Input[0] = '\0';
            reclaim = true;
        }
        ImGui::PopItemWidth();
        if (reclaim) ImGui::SetKeyboardFocusHere(-1);

        ImGui::End();
    }

    BOOM_INLINE void OnSelect(Entity) override { /* no-op */ }

    // ------------ API ------------
    BOOM_INLINE void Log(const std::string& s)
    {
        std::scoped_lock lk(m_Mu);
        if (m_Lines.size() == kMaxLines) m_Lines.pop_front();
        m_Lines.push_back(s);
        m_RequestScroll = true;
    }

    BOOM_INLINE void Register(const std::string& name,
        std::function<void(const std::string& args)> fn)
    {
        m_Cmds[name] = std::move(fn);
    }

    // Expose for menu toggle
    bool open = true;

private:
    // ------------ internals ------------
    BOOM_INLINE void Execute(const char* line)
    {
        std::string cmd = line; if (cmd.empty()) return;
        m_History.push_back(cmd); m_HistPos = -1;

        auto sp = cmd.find(' ');
        std::string name = (sp == std::string::npos) ? cmd : cmd.substr(0, sp);
        std::string args = (sp == std::string::npos) ? "" : cmd.substr(sp + 1);

        if (name == "help" || name == "clear")
        {
            // handled by built-ins through Register() at construction,
            // re-dispatch to keep behavior consistent
            if (auto it = m_Cmds.find(name); it != m_Cmds.end()) it->second(args);
            return;
        }

        if (auto it = m_Cmds.find(name); it != m_Cmds.end())
        {
            try { it->second(args); }
            catch (const std::exception& e) { Log(std::string("[error] ") + e.what()); }
        }
        else
        {
            Log("[unknown] " + name);
        }
    }

    static int HistoryCB(ImGuiInputTextCallbackData* data)
    {
        auto* self = static_cast<ConsoleWindow*>(data->UserData);
        if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
        {
            if (data->EventKey == ImGuiKey_UpArrow)
            {
                if (self->m_HistPos == -1) self->m_HistPos = (int)self->m_History.size() - 1;
                else if (self->m_HistPos > 0) --self->m_HistPos;
            }
            else if (data->EventKey == ImGuiKey_DownArrow)
            {
                if (self->m_HistPos != -1 && ++self->m_HistPos >= (int)self->m_History.size())
                    self->m_HistPos = -1;
            }
            const char* s = (self->m_HistPos == -1) ? "" : self->m_History[self->m_HistPos].c_str();
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, s);
        }
        return 0;
    }

private:
    static constexpr size_t kMaxLines = 4000;

    std::mutex m_Mu;
    std::deque<std::string> m_Lines;
    ImGuiTextFilter m_Filter;
    bool m_AutoScroll = true;
    bool m_RequestScroll = false;

    char m_Input[512]{};
    std::vector<std::string> m_History;
    int m_HistPos = -1;

    std::unordered_map<std::string, std::function<void(const std::string&)>> m_Cmds;
};
