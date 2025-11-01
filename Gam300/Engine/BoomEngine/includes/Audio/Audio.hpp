#pragma once

#include <fmod.hpp>
#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <glm/vec3.hpp>
#include "common/Core.h"

class SoundEngine {
public:
    static BOOM_API SoundEngine& Instance();

    BOOM_API bool  Init();
    BOOM_API void  Update();
    BOOM_API void  Shutdown();

    BOOM_API void  PlaySound(const std::string& name, const std::string& filePath, bool loop);
    BOOM_API void  StopSound(const std::string& name);
    BOOM_API void  SetVolume(const std::string& name, float volume);

    // NEW: 3D positional playback
    BOOM_API void  PlaySoundAt(const std::string& name, const std::string& filePath, const glm::vec3& position, bool loop);

    // NEW: helper to enumerate channel groups
    BOOM_API std::vector<std::string> GetChannelGroupNames() const;

    // NEW: route playback to named group (create group first or use existing)
    BOOM_API void  PlaySound(const std::string& name, const std::string& filePath, bool loop, const std::string& groupName);

    // NEW APIs
    BOOM_API bool  PreloadSound(const std::string& name, const std::string& filePath, bool stream = false, bool loop = false);
    BOOM_API void  UnloadSound(const std::string& name);

    BOOM_API void  SetGroupVolume(const std::string& groupName, float volume);
    BOOM_API float GetGroupVolume(const std::string& groupName) const;

    BOOM_API bool  CreateChannelGroup(const std::string& groupName, const std::string& parentGroup = "Master");
    BOOM_API bool  RemoveChannelGroup(const std::string& groupName);
    BOOM_API bool  HasChannelGroup(const std::string& groupName) const;

    BOOM_API bool  IsPlaying(const std::string& name) const;
    BOOM_API void  Pause(const std::string& name, bool pause);
    BOOM_API void  SetLooping(const std::string& name, bool loop);
    BOOM_API void  StopAllExcept(const std::string& keepName);

private:
    FMOD::System* mSystem = nullptr;
    std::unordered_map<std::string, FMOD::Sound*>   mSounds;
    std::unordered_map<std::string, FMOD::Channel*> mChannels;

    // new: named channel groups
    std::unordered_map<std::string, FMOD::ChannelGroup*> mChannelGroups;

    // ref counts for Preload/Unload
    std::unordered_map<std::string, int> mSoundRefCount;
    mutable std::mutex mMutex;
};