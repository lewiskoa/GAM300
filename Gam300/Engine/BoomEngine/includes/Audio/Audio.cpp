#include "Core.h"
#include "Audio.hpp"
#include <iostream>
#include <filesystem>

SoundEngine& SoundEngine::Instance() {
    static SoundEngine instance;
    return instance;
}

bool SoundEngine::Init() {
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
    return true;
}

void SoundEngine::Update() {
    if (mSystem) mSystem->update();
}

void SoundEngine::Shutdown() {
    for (auto& [name, sound] : mSounds) {
        sound->release();
    }
    mSounds.clear();
    if (mSystem) {
        mSystem->close();
        mSystem->release();
    }
}

void SoundEngine::PlaySound(const std::string& name, const std::string& filePath, bool loop)
{
    if (!mSystem) {
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

    channel->setVolume(1.0f);
    mChannels[name] = channel;
}


void SoundEngine::StopSound(const std::string& name) {
    if (mChannels.find(name) != mChannels.end()) {
        mChannels[name]->stop();
    }
}

void SoundEngine::SetVolume(const std::string& name, float volume) {
    if (mChannels.find(name) != mChannels.end()) {
        mChannels[name]->setVolume(volume);
    }
}

bool SoundEngine::IsPlaying(const std::string& name) const {
    auto it = mChannels.find(name);
    if (it == mChannels.end() || !it->second) return false;
    bool playing = false;
    it->second->isPlaying(&playing);
    return playing;
}

void SoundEngine::Pause(const std::string& name, bool pause) {
    auto it = mChannels.find(name);
    if (it != mChannels.end() && it->second) {
        it->second->setPaused(pause);
    }
}

void SoundEngine::SetLooping(const std::string& name, bool loop) {
    auto it = mChannels.find(name);
    if (it != mChannels.end() && it->second) {
        it->second->setMode(loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
    }
}

void SoundEngine::StopAllExcept(const std::string& keepName) {
    for (auto& [n, ch] : mChannels) {
        if (n != keepName && ch) ch->stop();
    }
}
