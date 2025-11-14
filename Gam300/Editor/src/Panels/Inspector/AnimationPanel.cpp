#include "Panels/Inspector/InspectorPanel.h"
#include "Editor.h"          // for Editor::GetContext()
#include "Context/Context.h"        // for Boom::AppContext + scene access
#include "Vendors/imgui/imgui.h"
#include "Auxiliaries/Assets.h"
#include "Context/DebugHelpers.h"


using namespace EditorUI;

namespace EditorUI {

    void InspectorPanel::AnimatorComponentUI(Boom::Entity& selected)
    {
        Boom::AppContext* ctx = GetContext();
        if (!ctx) return;
        if (!selected.Has<Boom::AnimatorComponent>()) return;

        ImGui::PushID("Animator");

        bool isOpen = ImGui::CollapsingHeader(
            "Animator",
            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap
        );

        const ImVec2 headerMin = ImGui::GetItemRectMin();
        const ImVec2 headerMax = ImGui::GetItemRectMax();
        const float  lineH = ImGui::GetFrameHeight();
        const float  y = headerMin.y + (headerMax.y - headerMin.y - lineH) * 0.5f;

        ImGui::SetCursorScreenPos(ImVec2(headerMax.x - lineH, y));
        if (ImGui::Button("...", ImVec2(lineH, lineH)))
            ImGui::OpenPopup("AnimatorSettings");

        bool removed = false;
        if (ImGui::BeginPopup("AnimatorSettings")) {
            if (ImGui::MenuItem("Remove Component")) {
                removed = true;
            }
            ImGui::EndPopup();
        }

        ImGui::SetCursorScreenPos(
            ImVec2(headerMin.x, headerMax.y + ImGui::GetStyle().ItemSpacing.y)
        );

        if (isOpen) {
            ImGui::Indent(12.0f);
            ImGui::Spacing();

            auto& animComp = selected.Get<Boom::AnimatorComponent>();
            auto& animator = animComp.animator;

            if (animator) {
                size_t clipCount = animator->GetClipCount();
                size_t stateCount = animator->GetStateCount();

                ImGui::Text("Clips: %zu", clipCount);
                ImGui::Text("States: %zu", stateCount);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // --- Add State button ---
                if (clipCount > 0) {
                    if (ImGui::Button("+ Add State", ImVec2(-1, 0))) {
                        std::string stateName = "State " + std::to_string(stateCount);
                        animator->AddState(stateName, 0);
                        BOOM_INFO("Added state '{}'", stateName);
                    }
                }
                else {
                    ImGui::BeginDisabled();
                    ImGui::Button("+ Add State (No clips loaded)", ImVec2(-1, 0));
                    ImGui::EndDisabled();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // --- States list ---
                size_t currentStateIdx = animator->GetCurrentStateIndex();
                if (stateCount > 0) {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "States");
                    ImGui::Spacing();
                }

                for (size_t i = 0; i < stateCount; ++i) {
                    const auto* state = animator->GetState(i);
                    if (!state) continue;

                    ImGui::PushID(static_cast<int>(i));

                    bool isCurrent = (i == currentStateIdx);
                    if (isCurrent) {
                        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.3f, 0.2f, 0.3f));
                    }

                    ImGui::BeginChild("StateItem", ImVec2(0, 100), true);

                    if (isCurrent) {
                        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s", state->name.c_str());
                    }
                    else {
                        ImGui::Text("%s", state->name.c_str());
                    }

                    const char* clipName = "None";
                    if (state->clipIndex < clipCount) {
                        const auto* clip = animator->GetClip(state->clipIndex);
                        if (clip) clipName = clip->name.c_str();
                    }
                    ImGui::TextDisabled("Clip: %s", clipName);

                    ImGui::Text(
                        "Speed: %.2f | Loop: %s",
                        state->speed,
                        state->loop ? "Yes" : "No"
                    );

                    ImGui::Spacing();

                    if (ImGui::Button("Edit", ImVec2(60, 0))) {
                        BOOM_INFO("Edit state '{}'", state->name);
                    }
                    ImGui::SameLine();

                    if (ImGui::Button("Remove", ImVec2(60, 0))) {
                        animator->RemoveState(i);
                        BOOM_INFO("Removed state at index {}", i);
                        ImGui::EndChild();
                        if (isCurrent) ImGui::PopStyleColor();
                        ImGui::PopID();
                        break;
                    }
                    ImGui::SameLine();

                    if (!isCurrent) {
                        if (ImGui::Button("Set Default", ImVec2(80, 0))) {
                            animator->SetDefaultState(i);
                            BOOM_INFO("Set '%s' as default state", state->name.c_str());
                        }
                    }
                    else {
                        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "[DEFAULT]");
                    }

                    ImGui::EndChild();

                    if (isCurrent) {
                        ImGui::PopStyleColor();
                    }

                    ImGui::PopID();
                    ImGui::Spacing();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
            }
            else {
                ImGui::TextDisabled("No animator available.");
                ImGui::Spacing();
            }

            ImGui::Unindent(12.0f);
        }

        if (removed) {
            ctx->scene.remove<Boom::AnimatorComponent>(m_App->SelectedEntity());
            ImGui::PopID();
            ImGui::Spacing();
            return;
        }

        ImGui::PopID();
        ImGui::Spacing();
    }



} // namespace EditorUI
