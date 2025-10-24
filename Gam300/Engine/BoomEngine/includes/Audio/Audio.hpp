#pragma once

#include <fmod.hpp>
#include <string>
#include <unordered_map>
#include "common/Core.h"

class  SoundEngine {
public:
    static BOOM_API SoundEngine& Instance();

    BOOM_API bool  Init();
    BOOM_API void  Update();
    BOOM_API void  Shutdown();

    BOOM_API void  PlaySound(const std::string& name, const std::string& filePath, bool loop);
    BOOM_API void  StopSound(const std::string& name);
    BOOM_API void  SetVolume(const std::string& name, float volume);

    // NEW: helpful for UI
    BOOM_API bool  IsPlaying(const std::string& name) const;
    BOOM_API void  Pause(const std::string& name, bool pause);
    BOOM_API  void  SetLooping(const std::string& name, bool loop);
    BOOM_API void  StopAllExcept(const std::string& keepName);

    BOOM_API bool  PreloadSound(const std::string& name, const std::string& filePath, bool stream = false, bool loop = false);
    BOOM_API void  UnloadSound(const std::string& name);

    BOOM_API bool  CreateChannelGroup(const std::string& groupName, const std::string& parentGroup = "Master"); 
    BOOM_API bool  RemoveChannelGroup(const std::string& groupName);
    BOOM_API bool  HasChannelGroup(const std::string& groupName) const;

    BOOM_API void  SetGroupVolume(const std::string& groupName, float volume);
    BOOM_API float GetGroupVolume(const std::string& groupName) const;


private:
    FMOD::System* mSystem = nullptr;
    std::unordered_map<std::string, FMOD::Sound*>   mSounds;
    std::unordered_map<std::string, FMOD::Channel*> mChannels;

    std::unordered_map<std::string, FMOD::ChannelGroup*> mChannelGroups;

    std::unordered_map<std::string, int> mSoundRefCount;
    mutable std::mutex mMutex;
};