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

    void SetMasterVolume(float volume);
    void SetMusicVolume(float volume);
    void SetSFXVolume(float volume);

    float GetMasterVolume() const;
    float GetMusicVolume() const;
    float GetSFXVolume() const;

    void SetListenerAttributes(const FMOD_VECTOR& pos, const FMOD_VECTOR& vel,
        const FMOD_VECTOR& forward, const FMOD_VECTOR& up);

    void SetSoundPosition(const std::string& name, const FMOD_VECTOR& pos, const FMOD_VECTOR& vel = { 0,0,0 });

private:
    SoundEngine() = default;
    ~SoundEngine() = default;

    FMOD::System* mSystem = nullptr;

    std::unordered_map<std::string, FMOD::Sound*> mSounds;
    std::unordered_map<std::string, FMOD::Channel*> mChannels;

    float mDopplerScale = 1.0f;
    float mDistanceFactor = 1.0f;
    float mRolloffScale = 1.0f;

    FMOD::ChannelGroup* mMasterGroup = nullptr;
    FMOD::ChannelGroup* mMusicGroup  = nullptr;
    FMOD::ChannelGroup* mSFXGroup    = nullptr;
};
