#pragma once
#include "Animation.h"
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace Boom
{
    struct Animator
    {
        BOOM_INLINE auto& Animate(float deltaTime)
        {
            if (m_CurrentClip < m_Clips.size())
            {
                m_Time += m_Clips[m_CurrentClip]->ticksPerSecond * deltaTime;
                m_Time = fmod(m_Time, m_Clips[m_CurrentClip]->duration);
                UpdateJoints(m_Root, glm::identity<glm::mat4>());
            }
            return m_Transforms;
        }

        // NEW: Switch animation at runtime
        BOOM_INLINE void PlayClip(size_t clipIndex)
        {
            if (clipIndex < m_Clips.size())
            {
                m_CurrentClip = clipIndex;
                m_Time = 0.0f; // Reset time when switching
            }
        }

        BOOM_INLINE void PlayClip(const std::string& clipName)
        {
            for (size_t i = 0; i < m_Clips.size(); ++i)
            {
                if (m_Clips[i]->name == clipName)
                {
                    PlayClip(i);
                    return;
                }
            }
        }

        // Getters
        BOOM_INLINE size_t GetCurrentClip() const { return m_CurrentClip; }
        BOOM_INLINE float GetTime() const { return m_Time; }
        BOOM_INLINE size_t GetClipCount() const { return m_Clips.size(); }
        BOOM_INLINE const AnimationClip* GetClip(size_t index) const
        {
            return (index < m_Clips.size()) ? m_Clips[index].get() : nullptr;
        }

        BOOM_INLINE size_t GetSequence() const { return GetCurrentClip(); }
        BOOM_INLINE void SetSequence(size_t index) { PlayClip(index); }

    private:
        BOOM_INLINE void GetPreviousAndNextFrames(
            const std::vector<KeyFrame>& keys,
            KeyFrame& prev,
            KeyFrame& next) const
        {
            if (keys.size() < 2)
            {
                if (!keys.empty()) prev = next = keys[0];
                return;
            }

            for (uint32_t i = 1; i < keys.size(); i++)
            {
                if (m_Time < keys[i].timeStamp)
                {
                    prev = keys[i - 1];
                    next = keys[i];
                    return;
                }
            }

            // If we're past the last keyframe (shouldn't happen with fmod)
            prev = next = keys.back();
        }

        BOOM_INLINE glm::mat4 Interpolate(const KeyFrame& prev, const KeyFrame& next, float progression)
        {
            return glm::translate(glm::mat4(1.0f), glm::mix(prev.position, next.position, progression)) *
                glm::toMat4(glm::normalize(glm::slerp(prev.rotation, next.rotation, progression))) *
                glm::scale(glm::mat4(1.0f), glm::mix(prev.scale, next.scale, progression));
        }

        BOOM_INLINE void UpdateJoints(Joint& joint, const glm::mat4& parentTransform)
        {
            glm::mat4 localTransform = glm::mat4(1.0f); // Default to identity

            // Get the current animation clip
            if (m_CurrentClip < m_Clips.size())
            {
                const AnimationClip* clip = m_Clips[m_CurrentClip].get();

                // Look up this joint's track in the current animation
                const auto* keys = clip->GetTrack(joint.name);

                if (keys && keys->size() >= 2)
                {
                    KeyFrame prev, next;
                    GetPreviousAndNextFrames(*keys, prev, next);

                    float progression = 0.0f;
                    float dt = next.timeStamp - prev.timeStamp;
                    if (dt > 0.0f)
                    {
                        progression = (m_Time - prev.timeStamp) / dt;
                    }

                    localTransform = Interpolate(prev, next, progression);
                }
                else if (keys && keys->size() == 1)
                {
                    // Only one keyframe - use it as static pose
                    const KeyFrame& key = (*keys)[0];
                    localTransform = glm::translate(glm::mat4(1.0f), key.position) *
                        glm::toMat4(key.rotation) *
                        glm::scale(glm::mat4(1.0f), key.scale);
                }
            }

            // Combine with parent transform
            glm::mat4 worldTransform = parentTransform * localTransform;

            // Update joint transform
            m_Transforms[joint.index] = worldTransform * m_GlobalTransform * joint.offset;

            // Update children
            for (auto& child : joint.children)
            {
                UpdateJoints(child, worldTransform);
            }
        }

    public:
        // For cloning
        BOOM_INLINE std::shared_ptr<Animator> Clone() const
        {
            auto clone = std::make_shared<Animator>();
            clone->m_GlobalTransform = m_GlobalTransform;
            clone->m_Clips = m_Clips; // Shared ownership of clips
            clone->m_Root = m_Root;
            clone->m_Transforms.resize(m_Transforms.size());
            clone->m_CurrentClip = m_CurrentClip;
            clone->m_Time = m_Time;
            return clone;
        }

        BOOM_INLINE void SetTime(float time)
        {
            m_Time = time;
            if (m_CurrentClip < m_Clips.size())
            {
                // Clamp to clip duration
                m_Time = std::min(m_Time, m_Clips[m_CurrentClip]->duration);
            }
        }

        BOOM_INLINE void LoadAnimationFromFile(const std::string& filepath, const std::string& clipName = "")
        {
            // Use Assimp to load just the animation data
            Assimp::Importer importer;
            const aiScene* ai_scene = importer.ReadFile(
                filepath.c_str(),
                aiProcess_LimitBoneWeights
            );

            if (!ai_scene || !ai_scene->mAnimations || ai_scene->mNumAnimations == 0)
            {
                BOOM_ERROR("Failed to load animation from: {}", filepath);
                return;
            }

            // Load the first animation from the file
            auto ai_anim = ai_scene->mAnimations[0];

            auto clip = std::make_shared<AnimationClip>();
            clip->name = clipName.empty() ? ai_anim->mName.C_Str() : clipName;
            clip->duration = (float)ai_anim->mDuration;
            clip->ticksPerSecond = (float)ai_anim->mTicksPerSecond;

            // Parse animation channels
            for (uint32_t j = 0; j < ai_anim->mNumChannels; j++)
            {
                aiNodeAnim* ai_channel = ai_anim->mChannels[j];
                std::string jointName(ai_channel->mNodeName.C_Str());

                std::vector<KeyFrame>& track = clip->tracks[jointName];

                uint32_t maxKeys = std::max({
                    ai_channel->mNumPositionKeys,
                    ai_channel->mNumRotationKeys,
                    ai_channel->mNumScalingKeys
                    });

                track.reserve(maxKeys);

                for (uint32_t k = 0; k < maxKeys; k++)
                {
                    KeyFrame key;

                    if (k < ai_channel->mNumPositionKeys)
                    {
                        key.position = AssimpToVec3(ai_channel->mPositionKeys[k].mValue);
                        key.timeStamp = (float)ai_channel->mPositionKeys[k].mTime;
                    }
                    if (k < ai_channel->mNumRotationKeys)
                    {
                        key.rotation = AssimpToQuat(ai_channel->mRotationKeys[k].mValue);
                    }
                    if (k < ai_channel->mNumScalingKeys)
                    {
                        key.scale = AssimpToVec3(ai_channel->mScalingKeys[k].mValue);
                    }

                    track.push_back(key);
                }
            }

            m_Clips.push_back(clip);
            BOOM_INFO("Loaded animation '{}' from {} - Duration: {:.2f}s",
                clip->name, filepath, clip->duration);
        }

    private:
        std::vector<std::shared_ptr<AnimationClip>> m_Clips{}; // Now we store clips, not raw Animation structs
        std::vector<glm::mat4> m_Transforms{};
        glm::mat4 m_GlobalTransform{};
        Joint m_Root;
        size_t m_CurrentClip = 0;
        float m_Time = 0.0f;

        friend struct SkeletalModel;
    };
}