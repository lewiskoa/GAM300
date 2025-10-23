#include "Core.h"
#include "Audio.hpp"
#include <iostream>
#include <filesystem>

static FMOD::ChannelGroup* sMasterGroup = nullptr;
static FMOD::ChannelGroup* sMusicGroup = nullptr;
static FMOD::ChannelGroup* sSFXGroup = nullptr;

static inline bool FMODCheck(FMOD_RESULT result, const char* context) {
    if (result != FMOD_OK) {
        std::cerr << "[FMOD] " << context << " failed: " << result << std::endl;
        return false;
    }
    return true;
}

BOOM_API SoundEngine& SoundEngine::Instance() {
    static SoundEngine instance;
    return instance;
}

BOOM_API bool SoundEngine::Init() {
    FMOD_RESULT result = FMOD::System_Create(&mSystem);
    if (result != FMOD_OK) {
        std::cerr << "FMOD error: " << result << std::endl;
        return false;
    }

    result = mSystem->init(512, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK) {
        std::cerr << "FMOD init failed: " << result << std::endl;
        return false;
    }

    result = mSystem->getMasterChannelGroup(&sMasterGroup);
    if (!FMODCheck(result, "getMasterChannelGroup")) return false;

    result = mSystem->createChannelGroup("Music", &sMusicGroup);
    if (!FMODCheck(result, "createChannelGroup Music")) return false;

    result = mSystem->createChannelGroup("SFX", &sSFXGroup);
    if (!FMODCheck(result, "createChannelGroup SFX")) return false;

    if (sMasterGroup && sMusicGroup) sMasterGroup->addGroup(sMusicGroup);
    if (sMasterGroup && sSFXGroup)   sMasterGroup->addGroup(sSFXGroup);

    return true;
}

BOOM_API void SoundEngine::Update() {
    if (mSystem) mSystem->update();

    for (auto it = mChannels.begin(); it != mChannels.end(); ) 
    {
        bool isPlaying = false;
        if (it->second) { it->second->isPlaying(&isPlaying); }

        if (!isPlaying) { it = mChannels.erase(it); } 

        else { ++it; }
	}
}

BOOM_API void SoundEngine::Shutdown() {
    for (auto& [name, ch] : mChannels) { if (ch) ch->stop(); }
    mChannels.clear();

    // Release sounds
    for (auto& [name, sound] : mSounds) { if (sound) sound->release(); }
    mSounds.clear();

    // Release channel groups
    if (sMusicGroup) { sMusicGroup->release(); sMusicGroup = nullptr; }
    if (sSFXGroup) { sSFXGroup->release();   sSFXGroup = nullptr; }
    sMasterGroup = nullptr; // master is owned by system

    if (mSystem) {
        mSystem->close();
        mSystem->release();
        mSystem = nullptr;
    }
}

BOOM_API void SoundEngine::PlaySound(const std::string& name, const std::string& filePath, bool loop)
{
    if (!mSystem) 
    {
        std::cerr << "FMOD system not initialized!\n";
        return;
    }

    std::cout << "[SoundEngine] Current working directory: "
        << std::filesystem::current_path().string() << std::endl;

    if (!std::filesystem::exists(filePath)) {
        std::cerr << "[SoundEngine] File does NOT exist: " << filePath << std::endl;
    }
    else {
        std::cout << "[SoundEngine] File found: " << filePath << std::endl;
    }

    if (mSounds.find(name) == mSounds.end()) {
        FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT | FMOD_2D;
        FMOD::Sound* sound = nullptr;

        FMOD_RESULT result = mSystem->createSound(filePath.c_str(), mode, nullptr, &sound);
        if (result != FMOD_OK) {
            std::cerr << "FMOD failed to load " << filePath << " error: " << result << std::endl;
            return;
        }

        mSounds[name] = sound;
    }

    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result = mSystem->playSound(mSounds[name], nullptr, false, &channel);
    if (result != FMOD_OK) {
        std::cerr << "FMOD failed to play " << name << " error: " << result << std::endl;
        return;
    }

    // Route looped audio to Music group, otherwise to SFX. (Heuristic)
    FMOD::ChannelGroup* targetGroup = loop ? sMusicGroup : sSFXGroup;

    if (channel && targetGroup) { channel->setChannelGroup(targetGroup); }

    // Set defaults and unpause
    if (channel) 
    {
        channel->setVolume(1.0f);
        channel->setPaused(false);
        mChannels[name] = channel;
    }
}


BOOM_API void SoundEngine::StopSound(const std::string& name) {
	auto it = mChannels.find(name);

    if (it != mChannels.end())
    {
        if (it->second) it->second->stop();
        mChannels.erase(it);
    }
}

BOOM_API void SoundEngine::SetVolume(const std::string& name, float volume) {
    auto it = mChannels.find(name);

    if (it != mChannels.end() && it->second) { it->second->setVolume(volume); }
}

BOOM_API bool SoundEngine::IsPlaying(const std::string& name) const {
    auto it = mChannels.find(name);
    if (it == mChannels.end() || !it->second) return false;
    bool playing = false;
    it->second->isPlaying(&playing);
    return playing;
}

BOOM_API void SoundEngine::Pause(const std::string& name, bool pause) {
    auto it = mChannels.find(name);

    if (it != mChannels.end() && it->second) { it->second->setPaused(pause); }
}

BOOM_API void SoundEngine::SetLooping(const std::string& name, bool loop) {
    auto it = mChannels.find(name);

    if (it != mChannels.end() && it->second) { it->second->setMode(loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF); }
}

BOOM_API void SoundEngine::StopAllExcept(const std::string& keepName) {
    for (auto it = mChannels.begin(); it != mChannels.end(); ) 
    {
        const std::string& n = it->first;
        FMOD::Channel* ch = it->second;
        if (n != keepName && ch) ch->stop();

        if (n != keepName)
            it = mChannels.erase(it);
        else
            ++it;
    }
}
