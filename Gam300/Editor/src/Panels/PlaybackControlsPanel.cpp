#include "Panels/PlaybackControlsPanel.h"
#include "Context/DebugHelpers.h"

PlaybackControlsPanel::PlaybackControlsPanel(AppInterface* ctx, Application* app)
    : IWidget(ctx)
    , m_Application(app)
{
    DEBUG_DLL_BOUNDARY("PlaybackControlsPanel::Constructor");
    DEBUG_POINTER(context, "AppInterface");
}

void PlaybackControlsPanel::Render()
{
    OnShow();
}

void PlaybackControlsPanel::OnShow()
{
    if (!m_ShowPlaybackControls) return;

    if (ImGui::Begin("Playback Controls", &m_ShowPlaybackControls))
    {
        Application* app = m_Application;

        if (app)
        {
            ApplicationState currentState = app->GetState();

            // Current state label
            ImGui::Text("Application State: ");
            ImGui::SameLine();
            switch (currentState)
            {
            case ApplicationState::RUNNING:
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "RUNNING");
                break;
            case ApplicationState::PAUSED:
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "PAUSED");
                break;
            case ApplicationState::STOPPED:
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "STOPPED");
                break;
            }

            ImGui::Separator();

            // Controls
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));

            // --- Play / Resume ---
            {
                bool canPlay = (currentState == ApplicationState::PAUSED ||
                    currentState == ApplicationState::STOPPED);

                if (!canPlay) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.6f, 0.0f, 1.0f));
                }

                if (ImGui::Button("Play/Resume", ImVec2(100, 30))) {
                    if (canPlay) {
                        app->Resume();
                        BOOM_INFO("[Editor] Play/Resume button clicked");
                    }
                }
                ImGui::PopStyleColor(3);
            }

            ImGui::SameLine();

            // --- Pause ---
            {
                bool canPause = (currentState == ApplicationState::RUNNING);

                if (!canPause) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 0.0f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
                }

                if (ImGui::Button("Pause", ImVec2(100, 30))) {
                    if (canPause) {
                        app->Pause();
                        BOOM_INFO("[Editor] Pause button clicked");
                    }
                }
                ImGui::PopStyleColor(3);
            }

            ImGui::SameLine();

            // --- Stop ---
            {
                bool canStop = (currentState != ApplicationState::STOPPED);

                if (!canStop) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
                }

                if (ImGui::Button("Stop", ImVec2(100, 30))) {
                    if (canStop) {
                        app->Stop();
                        BOOM_INFO("[Editor] Stop button clicked");
                    }
                }
                ImGui::PopStyleColor(3);
            }

            ImGui::PopStyleVar();

            ImGui::Separator();

            // Help / shortcuts
            ImGui::Text("Keyboard Shortcuts:");
            ImGui::BulletText("Spacebar: Toggle Pause/Resume");
            ImGui::BulletText("Escape: Stop Application");

            // Time info
            if (currentState != ApplicationState::STOPPED) {
                ImGui::Separator();
                ImGui::Text("Adjusted Time: %.2f seconds", app->GetAdjustedTime());

                if (currentState == ApplicationState::PAUSED) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Time is paused");
                }
            }
        }
        else
        {
            ImGui::Text("Application reference not available");
        }
    }
    ImGui::End();
}
