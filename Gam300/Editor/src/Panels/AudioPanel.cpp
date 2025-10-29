#include "Panels/AudioPanel.h"
#include "Vendors/imgui/imgui.h"


namespace EditorUI
{
    AudioPanel::AudioPanel(AppInterface* ctx)
        : IWidget(ctx)
        , m_Tracks{
            // Same list style as your namespace snippet
            Track{ "Menu",   "Resources/Audio/Fetty Wap.wav" },
            // Add more here if you want:
            Track{ "BOOM",   "Resources/Audio/vboom.wav" },
            Track{ "Fish",   "Resources/Audio/FISH.wav" },
            Track{ "Ambi",   "Resources/Audio/outdoorAmbience.wav" },
            Track{ "Schizo", "Resources/Audio/the voices.wav" }
        }
    {
        EnsureVolumeKeys();
    }

    void AudioPanel::Render()
    {
        OnShow();
    }

    void AudioPanel::OnShow()
    {
        if (!m_Show) return;

        auto& audio = SoundEngine::Instance();
        EnsureVolumeKeys();

        if (ImGui::Begin("Music", &m_Show))
        {
            // ----- Track picker (exact behavior) -----
            if (!m_Tracks.empty())
            {
                const char* current = m_Tracks[m_Selected].name.c_str();
                if (ImGui::BeginCombo("Track", current))
                {
                    for (int i = 0; i < static_cast<int>(m_Tracks.size()); ++i)
                    {
                        bool isSel = (i == m_Selected);
                        if (ImGui::Selectable(m_Tracks[i].name.c_str(), isSel))
                            m_Selected = i;
                        if (isSel) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }

            const std::string& name = m_Tracks[m_Selected].name;
            const std::string& path = m_Tracks[m_Selected].path;

            // ----- Loop + Restart -----
            if (ImGui::Checkbox("Loop", &m_Loop)) {
                audio.SetLooping(name, m_Loop);
            }
            ImGui::SameLine();
            if (ImGui::Button("Restart")) {
                audio.StopAllExcept("");
                audio.PlaySound(name, path, m_Loop);
                audio.SetVolume(name, m_Volume[name]);
                m_Paused = false;
            }

            // ----- Volume -----
            float vol = m_Volume[name];
            if (ImGui::SliderFloat("Volume", &vol, 0.0f, 1.0f, "%.2f")) {
                m_Volume[name] = vol;
                audio.SetVolume(name, vol);
            }

            // ----- Play / Stop / Pause -----
            const bool playing = audio.IsPlaying(name);
            if (!playing) {
                if (ImGui::Button("Play")) {
                    audio.StopAllExcept("");
                    audio.PlaySound(name, path, m_Loop);
                    audio.SetVolume(name, m_Volume[name]);
                    m_Paused = false;
                }
            }
            else {
                if (ImGui::Button("Stop")) {
                    audio.StopSound(name);
                    m_Paused = false;
                }
                ImGui::SameLine();
                if (ImGui::Checkbox("Paused", &m_Paused)) {
                    audio.Pause(name, m_Paused);
                }
            }

            // ----- Quick Switch -----
            ImGui::SeparatorText("Quick Switch");
            for (int i = 0; i < static_cast<int>(m_Tracks.size()); ++i) {
                ImGui::PushID(i);
                if (ImGui::Button(m_Tracks[i].name.c_str())) {
                    m_Selected = i;
                    const auto& n = m_Tracks[i].name;
                    const auto& p = m_Tracks[i].path;

                    audio.StopAllExcept("");
                    audio.PlaySound(n, p, m_Loop);
                    audio.SetVolume(n, m_Volume[n]);
                    m_Paused = false;
                }
                ImGui::PopID();
                if ((i % 3) != 2) ImGui::SameLine();
            }
        }
        ImGui::End();
    }

    void AudioPanel::EnsureVolumeKeys()
    {
        for (const auto& t : m_Tracks) {
            if (!m_Volume.count(t.name))
                m_Volume[t.name] = 1.0f;
        }
    }
} // namespace EditorUI
