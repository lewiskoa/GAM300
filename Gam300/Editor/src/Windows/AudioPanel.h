// Editor/src/Windows/Audio.h
#pragma once
#include "Context/Context.h"
#include "Context/DebugHelpers.h"
#include "Vendors/imgui/imgui.h"

#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "Audio/Audio.hpp"

namespace EditorUI::Audio
{
    // Call this every frame somewhere in your editor UI pass
    inline void Render()
    {
        auto& audio = SoundEngine::Instance();
        
        // Catalog (adjust to your paths)
        static const std::vector<std::pair<std::string, std::string>> kTracks = {
            { "Menu", "Resources/Audio/Fetty Wap.wav" },

            
        };

        // State
        static int  selected = 0;
        bool loop = false;
        static std::unordered_map<std::string, float> sVolume;
        for (auto& [n, _] : kTracks) if (!sVolume.count(n)) sVolume[n] = 1.0f;

        if (ImGui::Begin("Music"))
        {
            // Track picker
            if (ImGui::BeginCombo("Track", kTracks[selected].first.c_str()))
            {
                for (int i = 0; i < (int)kTracks.size(); ++i)
                {
                    bool isSel = (i == selected);
                    if (ImGui::Selectable(kTracks[i].first.c_str(), isSel)) selected = i;
                    if (isSel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            const std::string& name = kTracks[selected].first;
            const std::string& path = kTracks[selected].second;

            // Loop + Restart
            if (ImGui::Checkbox("Loop", &loop)) {
                audio.SetLooping(name, loop);
            }
            ImGui::SameLine();
            if (ImGui::Button("Restart")) {
                audio.StopAllExcept("");
                audio.PlaySound(name, path, loop);
                audio.SetVolume(name, sVolume[name]);
            }

            // Volume
            float vol = sVolume[name];
            if (ImGui::SliderFloat("Volume", &vol, 0.0f, 1.0f, "%.2f")) {
                sVolume[name] = vol;
                audio.SetVolume(name, vol);
            }

            // Playback
            bool playing = audio.IsPlaying(name);
            if (!playing) {
                if (ImGui::Button("Play")) {
                    audio.StopAllExcept("");
                    audio.PlaySound(name, path, loop);
                    audio.SetVolume(name, sVolume[name]);
                }
            }
            else {
                if (ImGui::Button("Stop")) {
                    audio.StopSound(name);
                }
                ImGui::SameLine();
                static bool paused = false;
                if (ImGui::Checkbox("Paused", &paused)) {
                    audio.Pause(name, paused);
                }
            }

            // Quick switch (optional)
            ImGui::SeparatorText("Quick Switch");
            for (int i = 0; i < (int)kTracks.size(); ++i) {
                ImGui::PushID(i);
                if (ImGui::Button(kTracks[i].first.c_str())) {
                    selected = i;
                    audio.StopAllExcept("");
                    audio.PlaySound(kTracks[i].first, kTracks[i].second, loop);
                    audio.SetVolume(kTracks[i].first, sVolume[kTracks[i].first]);
                }
                ImGui::PopID();
                if ((i % 3) != 2) ImGui::SameLine();
            }
        }
        ImGui::End();
    }
} // namespace EditorUI::Audio
