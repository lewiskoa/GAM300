#pragma once
#include "Auxiliaries/Profiler.h"
#include "Vendors/imgui/imgui.h"
#include "Core.h"

BOOM_INLINE void DrawProfilerPanel(Boom::Profiler& prof) {
    const auto rows = prof.Snapshot();
    const float total = prof.SnapshotTotalMs();

    if (ImGui::Begin("Profiler")) {
        ImGui::Text("Total (flat): %.3f ms", total);
        ImGui::Separator();

        if (ImGui::BeginTable("prof_tbl", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Zone");
            ImGui::TableSetupColumn("Last (ms)", ImGuiTableColumnFlags_WidthFixed, 90.f);
            ImGui::TableSetupColumn("%", ImGuiTableColumnFlags_WidthFixed, 60.f);
            ImGui::TableHeadersRow();

            for (const auto& r : rows) {
                const float ms = r.data.lastFrameTime;
                const float pct = (total > 0.f) ? (ms / total * 100.f) : 0.f;
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(r.name.c_str());
                ImGui::TableSetColumnIndex(1); ImGui::Text("%.3f", ms);
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.1f", pct);
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();
}
