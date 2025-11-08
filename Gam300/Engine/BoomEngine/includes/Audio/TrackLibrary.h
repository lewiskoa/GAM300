#pragma once
#include <string>
#include <vector>

namespace Boom {

    struct Track { std::string name; std::string path; };

    // Return a static, built-in list (same as your AudioPanel ctor)
    inline const std::vector<Track>& GetBuiltinTracks()
    {
        static const std::vector<Track> kTracks = {
            { "Menu",   "Resources/Audio/Fetty Wap.wav" },
            { "BOOM",   "Resources/Audio/vboom.wav"     },
            { "Fish",   "Resources/Audio/FISH.wav"      },
            { "Ambi",   "Resources/Audio/outdoorAmbience.wav" },
            { "Schizo", "Resources/Audio/the voices.wav" }
        };
        return kTracks;
    }

    inline int FindTrackIndexByName(const std::vector<Track>& tracks, const std::string& name)
    {
        for (int i = 0; i < (int)tracks.size(); ++i)
            if (tracks[(size_t)i].name == name) return i;
        return -1;
    }

} // namespace Boom
