#pragma once
#include "Animation.h"

namespace Boom
{
     /**
     * @brief Drives skeletal animation by advancing time and updating joint transforms.
     *
     * The Animator owns a list of @c Animation clips and a transform buffer (one
     * matrix per joint). Calling Animate() advances the local time of the current
     * clip (indexed by @c m_Sequence), then performs a depth-first traversal from
     * the root joint to compute interpolated local transforms and write final
     * skinned (object-space) matrices into @c m_Transforms.
     *
     * @note Thread-safety: not thread-safe; mutates internal time and buffers.
     * @note Time units: @p deltaTime is expected in seconds.
     */
	struct Animator 
	{
        /**
         * @brief Advance the active animation and update all joint transforms.
         *
         * If @c m_Sequence is a valid index into @c m_Animations, the internal clock
         * @c m_Time is advanced by @c speed * deltaTime and wrapped into the clip
         * duration (looping). Then the skeleton is updated via @ref UpdateJoints.
         * If the sequence is invalid, no work is performed and the current transform
         * buffer is returned unchanged.
         *
         * @param[in] deltaTime Elapsed time (seconds) since the previous update.
         *
         * @return std::vector<glm::mat4>& Reference to the per-joint final transform
         *         buffer (index = joint index).
         *
         * @pre @c m_Animations[m_Sequence] exists and its joints' keyframes cover
         *      the full duration with at least two ordered keys per joint.
         * @post @c m_Transforms contains updated matrices for all joints.
         *
         * @remarks Complexity: O(J) where J is the number of joints (one DFS).
         */
		BOOM_INLINE auto& Animate(float deltaTime) 
		{
            if (m_Sequence < m_Animations.size())
            {
                m_Time += m_Animations[m_Sequence].speed *
                    deltaTime;
                m_Time = fmod(m_Time,
                    m_Animations[m_Sequence].duration);
                UpdateJoints(m_Root, glm::identity<glm::mat4>
                    ());
            }
            return m_Transforms;
        }

		// Getters and setters
    public:
        BOOM_INLINE int32_t GetSequence() const { return m_Sequence; }
        BOOM_INLINE float GetTime() const { return m_Time; }
        BOOM_INLINE void SetSequence(int32_t sequence) { m_Sequence = sequence; }
        BOOM_INLINE void SetTime(float time) { m_Time = time; }

    private:
        /**
        * @brief Interpolate between two keyframes to produce a local joint matrix.
        *
        * Performs standard TRS interpolation:
        *  - position: linear interpolation (lerp)
        *  - rotation: normalized spherical linear interpolation (slerp)
        *  - scale:    linear interpolation (lerp)
        *
        * @param[in] prev        Previous keyframe (t0).
        * @param[in] next        Next keyframe (t1), with t1 > t0.
        * @param[in] progression Normalized time in [0, 1] between @p prev and @p next.
        *
        * @return glm::mat4 The local transform matrix at the interpolated time.
        *
        * @pre 0.0f <= @p progression <= 1.0f.
        * @note Assumes right-handed TRS composition: translate * rotate * scale.
        */
        //BOOM_INLINE glm::mat4 Interpolate(const KeyFrame& prev, const KeyFrame& next, float progression)
        //{
        //    return (glm::translate(glm::mat4(1.0f),
        //        glm::mix(prev.position, next.position, progression)) *

        //        glm::toMat4(glm::normalize(glm::slerp(prev.rotation
        //            , next.rotation, progression))) *
        //        glm::scale(glm::mat4(1.0f), glm::mix(prev.scale,
        //            next.scale, progression)));
        //}

        /**
         * @brief Find the two keyframes bracketing the current local time for a joint.
         *
         * Scans the joint's sorted key list and returns the last key with timestamp
         * <= @c m_Time as @p prev and the first key with timestamp > @c m_Time as @p next.
         *
         * @param[in]  joint Joint whose keyframes are queried (expects sorted @c keys).
         * @param[out] prev  Output previous keyframe.
         * @param[out] next  Output next keyframe.
         *
         * @pre @c joint.keys.size() >= 2 and timestamps are strictly increasing and
         *      cover the clip range. @c m_Time is wrapped into the clip duration.
         * @warning No explicit fallback is provided if @c m_Time is beyond the last key
         *          (should not happen because @c Animate() wraps time with @c fmod).
         */
        //BOOM_INLINE void GetPreviousAndNextFrames(Joint&
        //    joint, KeyFrame& prev, KeyFrame& next)
        //{
        //    for (uint32_t i = 1u; i < joint.keys.size(); i++)
        //    {
        //        if (m_Time < joint.keys[i].timeStamp)
        //        {
        //            prev = joint.keys[i - 1];
        //            next = joint.keys[i];
        //            return;
        //        }
        //    }
        //}


        // Replace GetPreviousAndNextFrames and Interpolate with:
        BOOM_INLINE glm::mat4 EvaluateJointTransform(Joint& joint, float time)
        {
            auto frame = joint.curve.Evaluate(time);
            return glm::translate(glm::mat4(1.0f), frame.position) *
                glm::toMat4(frame.rotation) *
                glm::scale(glm::mat4(1.0f), frame.scale);
        }

        /**
        * @brief Recursively compute and store final joint transforms.
        *
        * Builds the current joint's local transform by interpolating between the
        * bracketing keyframes, composes it with the parent's transform, then writes
        * the final skinned matrix into @c m_Transforms at @c joint.index:
        *
        * final = parent * local * m_GlobalTransform * joint.offset
        *
        * After updating the current joint, the function recurses into each child.
        *
        * @param[in] joint            The joint to update (and recurse into children).
        * @param[in] parentTransform  The accumulated transform from the parent chain.
        *
        * @pre @c m_Transforms has size >= max joint index + 1.
        * @post @c m_Transforms[joint.index] holds the current frame's final matrix.
        *
        * @remarks Depth-first traversal; each joint visited once.
        */
        BOOM_INLINE void UpdateJoints(Joint& joint, const glm::mat4& parentTransform)
        {
            // Use the new curve-based evaluation
            glm::mat4 localTransform = EvaluateJointTransform(joint, m_Time);

            // Combine with parent transform
            glm::mat4 worldTransform = parentTransform * localTransform;

            // Update joint transform
            m_Transforms[joint.index] = worldTransform * m_GlobalTransform * joint.offset;

            // Update children joints
            for (auto& child : joint.children) {
                UpdateJoints(child, worldTransform);
            }
        }

    private:
        std::vector<Animation> m_Animations{};   //!< Animation clips (durations, speeds, joint keys).
        std::vector<glm::mat4> m_Transforms{};   //!< Final per-joint transforms (skinning matrices).
        glm::mat4 m_GlobalTransform{};           //!< Global/model transform applied to all joints.
        friend struct SkeletalModel;           //!< Grants SkeletalModel access to internals.
        int32_t m_Sequence = 0;                //!< index of the active animation clip.
        float m_Time = 0.0f;                   //!< Local time within the active clip (seconds, wrapped).
        Joint m_Root;                          //!< Skeleton root joint.
	};
}