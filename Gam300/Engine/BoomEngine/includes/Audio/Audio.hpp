#pragma once
#include <fmod.hpp>
#include <string>
#include <unordered_map>
#include "common/Core.h"

class SoundEngine {
public:
    static SoundEngine& Instance();

    bool Init();
    void Update();
    void Shutdown();

    void PlaySound(const std::string& name, const std::string& filePath, bool loop = false);
    void StopSound(const std::string& name);
    void SetVolume(const std::string& name, float volume);

private:
    SoundEngine() = default;
    ~SoundEngine() = default;

    FMOD::System* mSystem = nullptr;
    std::unordered_map<std::string, FMOD::Sound*> mSounds;
    std::unordered_map<std::string, FMOD::Channel*> mChannels;
};
