#include "Panels/PerformancePanel.h"
#include "Editor.h"
#include "Context/Context.h"
#include "Context/DebugHelpers.h"
#include "Context/Profiler.hpp"     // DrawProfilerPanel(..)
#include "Vendors/imgui/imgui.h"

namespace EditorUI {

    PerformancePanel::PerformancePanel(Editor* owner)
        : m_Owner(owner)
    {
        DEBUG_DLL_BOUNDARY("PerformancePanel::Ctor");
        if (!m_Owner) { BOOM_ERROR("PerformancePanel - null owner"); return; }
        m_Ctx = m_Owner->GetContext();
        DEBUG_POINTER(m_Ctx, "AppContext");
    }

    void PerformancePanel::Render() { OnShow(); }

    void PerformancePanel::OnShow()
    {
        if (!m_Show) return;

        if (ImGui::Begin("Performance", &m_Show))
        {
            ImGuiIO& io = ImGui::GetIO();
            const float fps = io.Framerate;
            const float ms = fps > 0.f ? 1000.f / fps : 0.f;

            ImGui::Text("FPS: %.1f  (%.2f ms)", fps, ms);
            ImGui::Separator();

            // Update history
            m_FpsHistory[m_FpsWriteIdx] = fps;
            m_FpsWriteIdx = (m_FpsWriteIdx + 1) % kPerfHistory;

            float temp[kPerfHistory];
            for (int i = 0; i < kPerfHistory; ++i)
                temp[i] = m_FpsHistory[(m_FpsWriteIdx + i) % kPerfHistory];

            ImVec2 plotSize(ImGui::GetContentRegionAvail().x, 80.f);
            ImGui::PlotLines("FPS", temp, kPerfHistory, 0, nullptr, 0.0f, 240.0f, plotSize);

            if (fps >= 120.f)      ImGui::TextColored(ImVec4(0.3f, 1, 0.3f, 1), "Very fast");
            else if (fps >= 60.f)  ImGui::TextColored(ImVec4(0.6f, 1, 0.6f, 1), "Good");
            else if (fps >= 30.f)  ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "Playable");
            else                   ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Slow");

            // Profiler integration - FIXED: Check pointer, not reference
            if (m_Ctx && m_Ctx->profiler != nullptr) {
                DrawProfilerPanel(m_Ctx->profiler);
            }
            else {
                ImGui::TextDisabled("Profiler unavailable");
            }
        }
        ImGui::End();
    }

} // namespace EditorUI