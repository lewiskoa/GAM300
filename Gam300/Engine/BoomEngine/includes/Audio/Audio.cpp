#include "Core.h"
#include "Audio.hpp"
#include <iostream>

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

void SoundEngine::PlaySound(const std::string& name, const std::string& filePath, bool loop) {
    if (!mSystem) return;

    if (mSounds.find(name) == mSounds.end()) {
        FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT;
        FMOD::Sound* sound = nullptr;
        mSystem->createSound(filePath.c_str(), mode, nullptr, &sound);
        mSounds[name] = sound;
    }

    FMOD::Channel* channel = nullptr;
    mSystem->playSound(mSounds[name], nullptr, false, &channel);
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
