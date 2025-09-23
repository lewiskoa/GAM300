#pragma once
#include "Helper.h"

namespace Boom
{
    /**
     * @brief Animation clip metadata.
     *
     * Describes a named clip with total duration and a playback speed multiplier.
     * The clip's per-joint curves live on each @ref Joint as a list of @ref KeyFrame entries
     * (one track per joint), typically sorted by @ref KeyFrame::timeStamp.
     *
     * @note duration is expressed in seconds after any ticks-per-second conversion during import.
     * @see Boom::Joint, Boom::KeyFrame, Boom::Animator
     */
    struct Animation
    {
        /** @brief Total clip length in seconds (post-import). */
        float duration = 0.0f;

        /**
        * @brief Playback speed multiplier.
        *
        * 1.0 = authored speed, 0.5 = half speed (slow motion), 2.0 = double speed.
        * The effective sampling time is usually: @code t_effective = t * speed; @endcode
        */
        float speed = 1.0f;

        /** @brief Human-readable clip name (e.g., "Walk", "Idle", "Run"). */
        std::string name;
    };



    enum class InterpolationMode
    {
        Linear,
        Constant,
        Bezier
    };

    /**
         * @brief A single sampled pose for a joint (bone) at a moment in time.
         *
         * Engines typically *interpolate* between adjacent keyframes during evaluation:
         * - Positions and scales use linear interpolation.
         * - Rotations use normalized quaternion SLERP.
         *
         * All transforms are **local to the joint's parent** (not world space).
         *
         * @note timeStamp is expected in seconds and should be monotonically non-decreasing per track.
         * @warning The identity quaternion is `glm::quat(1, 0, 0, 0)`; ensure your defaults/loads produce a valid unit quat.
         * @see Boom::Animator, Boom::Joint
         */
    struct KeyFrame
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::vec3(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);
        float timeStamp = 0.0f;
        InterpolationMode mode = InterpolationMode::Linear;

        // For bezier interpolation (optional tangent control)
        glm::vec3 positionOutTangent = glm::vec3(0.0f);
        glm::vec3 positionInTangent = glm::vec3(0.0f);
    };


    /**
    * @brief Simple animation curve for a joint's transform
    */
    class JointCurve
    {
    public:
        std::vector<KeyFrame> keys;

        BOOM_INLINE void AddKey(float time, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale,
            InterpolationMode mode = InterpolationMode::Linear)
        {
            KeyFrame key;
            key.timeStamp = time;
            key.position = pos;
            key.rotation = rot;
            key.scale = scale;
            key.mode = mode;

            // Insert in sorted order
            auto it = std::lower_bound(keys.begin(), keys.end(), key,
                [](const KeyFrame& a, const KeyFrame& b) {
                    return a.timeStamp < b.timeStamp;
                });
            keys.insert(it, key);
        }

        BOOM_INLINE KeyFrame Evaluate(float time) const
        {
            if (keys.empty()) return KeyFrame{};
            if (keys.size() == 1) return keys[0];

            // Handle time outside bounds
            if (time <= keys.front().timeStamp) return keys.front();
            if (time >= keys.back().timeStamp) return keys.back();

            // Find surrounding keyframes
            auto it = std::lower_bound(keys.begin(), keys.end(), time,
                [](const KeyFrame& key, float t) {
                    return key.timeStamp < t;
                });

            if (it == keys.begin()) return *it;

            const auto& nextKey = *it;
            const auto& prevKey = *(it - 1);

            float t = (time - prevKey.timeStamp) / (nextKey.timeStamp - prevKey.timeStamp);

            KeyFrame result;
            result.timeStamp = time;

            switch (prevKey.mode)
            {
            case InterpolationMode::Constant:
                result.position = prevKey.position;
                result.rotation = prevKey.rotation;
                result.scale = prevKey.scale;
                break;

            case InterpolationMode::Linear:
                result.position = glm::mix(prevKey.position, nextKey.position, t);
                result.rotation = glm::slerp(prevKey.rotation, nextKey.rotation, t);
                result.scale = glm::mix(prevKey.scale, nextKey.scale, t);
                break;

            case InterpolationMode::Bezier:
                // Simple bezier with automatic tangents for now
                result.position = EvaluateBezierPosition(prevKey, nextKey, t);
                result.rotation = glm::slerp(prevKey.rotation, nextKey.rotation, t); // Linear for rotation
                result.scale = glm::mix(prevKey.scale, nextKey.scale, t);
                break;
            }

            return result;
        }

    private:
        BOOM_INLINE glm::vec3 EvaluateBezierPosition(const KeyFrame& prev, const KeyFrame& next, float t) const
        {
            // Simple bezier interpolation
            float dt = next.timeStamp - prev.timeStamp;
            glm::vec3 p0 = prev.position;
            glm::vec3 p1 = prev.position + prev.positionOutTangent * dt * 0.33333f;
            glm::vec3 p2 = next.position - next.positionInTangent * dt * 0.33333f;
            glm::vec3 p3 = next.position;

            float u = 1.0f - t;
            float tt = t * t;
            float uu = u * u;
            float uuu = uu * u;
            float ttt = tt * t;

            return uuu * p0 + 3 * uu * t * p1 + 3 * u * tt * p2 + ttt * p3;
        }
    };

    /**
    * @brief A node in the skeleton hierarchy (a.k.a. bone/joint).
    *
    * Each joint may have:
    * - children that form the tree/graph of the skeleton.
    * - A set of animation @ref KeyFrame samples (its track) for the currently loaded clip.
    * - An inverse bind (offset) matrix used to transform skinned vertices from model space
    *   into joint space at bind pose for skinning.
    *
    * During skinning on CPU/GPU, the typical pipeline is:
    * 1) Evaluate each joint's local TRS at time *t* via interpolation of its @ref keys.
    * 2) Accumulate to a global (model-space) matrix along the parent chain.
    * 3) Multiply by @ref offset (inverse bind) to get the final skinning matrix for that joint.
    *
    * @note @ref index maps the joint to a slot in the shader's joint matrix array (e.g., `u_Joints[index]`).
    * @see Boom::Animation, Boom::KeyFrame
    */
    struct Joint
    {
        std::vector<Joint> children{};
        //std::vector<KeyFrame> keys{};
		JointCurve curve{};
        std::string name{};
        glm::mat4 offset{}; // <- inverse transform mtx
        int32_t index{};
    };

}