#include "Panels/PerformancePanel.h"
#include "Editor/EditorPCH.h"

// Optional include if DrawProfilerPanel is declared elsewhere
#include "Context/Profiler.hpp"   // Provides DrawProfilerPanel(m_Context->profiler)

PerformancePanel::PerformancePanel(AppInterface* ctx)
    : IWidget(ctx)
{
    DEBUG_DLL_BOUNDARY("PerformancePanel::Constructor");
    DEBUG_POINTER(context, "AppInterface");

    // Initialize FPS history with neutral values
    for (float& v : m_FpsHistory)
        v = 0.0f;

    BOOM_INFO("PerformancePanel::Constructor - Initialized FPS buffer with {} samples", kPerfHistory);
}

void PerformancePanel::Render()
{
    OnShow();
}

void PerformancePanel::OnShow()
{
    if (!m_ShowPerformance) return;

    if (ImGui::Begin("Performance", &m_ShowPerformance))
    {
        ImGuiIO& io = ImGui::GetIO();
        const float fps = io.Framerate;
        const float ms = fps > 0.f ? 1000.f / fps : 0.f;

        // --- Basic info ---
        ImGui::Text("FPS: %.1f  (%.2f ms)", fps, ms);
        ImGui::Separator();

        // --- History buffer update ---
        m_FpsHistory[m_FpsWriteIdx] = fps;
        m_FpsWriteIdx = (m_FpsWriteIdx + 1) % kPerfHistory;

        float temp[kPerfHistory];
        for (int i = 0; i < kPerfHistory; ++i)
            temp[i] = m_FpsHistory[(m_FpsWriteIdx + i) % kPerfHistory];

        // --- Plot FPS ---
        ImVec2 plotSize(ImGui::GetContentRegionAvail().x, 80.f);
        ImGui::PlotLines("FPS", temp, kPerfHistory, 0, nullptr, 0.0f, 240.0f, plotSize);

        // --- Status text ---
        if (fps >= 120.f)
            ImGui::TextColored(ImVec4(0.3f, 1, 0.3f, 1), "Very fast");
        else if (fps >= 60.f)
            ImGui::TextColored(ImVec4(0.6f, 1, 0.6f, 1), "Good");
        else if (fps >= 30.f)
            ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "Playable");
        else
            ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Slow");

        

        // --- Profiler integration ---
        if (m_Context && m_Context->profiler)
        {
            DrawProfilerPanel(m_Context->profiler);
        }
        else
        {
            ImGui::TextDisabled("Profiler unavailable");
        }
    }
    ImGui::End();
}
