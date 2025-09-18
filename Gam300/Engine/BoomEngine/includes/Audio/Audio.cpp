#include "Core.h"
#include "Audio.hpp"
#include <iostream>
#include <filesystem>

void SoundEngine::SetMasterVolume(float volume) {
    if (mMasterGroup) {
        mMasterGroup->setVolume(volume);
    }
}

void SoundEngine::SetMusicVolume(float volume) {
    if (mMusicGroup) {
        mMusicGroup->setVolume(volume);
    }
}

void SoundEngine::SetSFXVolume(float volume) {
    if (mSFXGroup) {
        mSFXGroup->setVolume(volume);
    }
}

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

    mSystem->set3DSettings(mDopplerScale, mDistanceFactor, mRolloffScale);

    result = mSystem->createChannelGroup("Master", &mMasterGroup);
    if (result != FMOD_OK) { std::cerr << "FMOD error creating Master group\n"; return false; }

    result = mSystem->createChannelGroup("Music", &mMusicGroup);
    if (result != FMOD_OK) { std::cerr << "FMOD error creating Music group\n"; return false; }

    result = mSystem->createChannelGroup("SFX", &mSFXGroup);
    if (result != FMOD_OK) { std::cerr << "FMOD error creating SFX group\n"; return false; }

    mMasterGroup->addGroup(mMusicGroup);
    mMasterGroup->addGroup(mSFXGroup);

    return true;
}

void SoundEngine::Update() { if (mSystem) mSystem->update();}

void SoundEngine::Shutdown() {
    for (auto& [name, sound] : mSounds) { sound->release();}

    mSounds.clear();

    if (mSystem) {
        mSystem->close();
        mSystem->release();
    }

    mMasterGroup = nullptr;
    mMusicGroup = nullptr;
    mSFXGroup = nullptr;
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
        //FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT | FMOD_2D; Use only for UI/BGM (I will figure it out later)
        FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL | FMOD_3D : FMOD_3D;
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

    if (loop && mMusicGroup)
    {
        channel->setChannelGroup(mMusicGroup);
    }
    else if (mSFXGroup)
    {
        channel->setChannelGroup(mSFXGroup);
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

void SoundEngine::SetListenerAttributes(const FMOD_VECTOR& pos, const FMOD_VECTOR& vel,
    const FMOD_VECTOR& forward, const FMOD_VECTOR& up) {
    if (mSystem) {
        mSystem->set3DListenerAttributes(0, &pos, &vel, &forward, &up);
    }
}

void SoundEngine::SetSoundPosition(const std::string& name, const FMOD_VECTOR& pos, const FMOD_VECTOR& vel) {
    if (mChannels.find(name) != mChannels.end()) {
        mChannels[name]->set3DAttributes(&pos, &vel);
    }
}

