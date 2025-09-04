#pragma once
#include "Helper.h"

namespace Boom
{
    /**
     * @brief Animation clip metadata.
     *
     * Describes a named clip with total duration and a playback speed multiplier.
     * The clip's per-joint curves live on each @ref Joint as a list of @ref KeyFrame entries
     * (one track per joint), typically sorted by @ref KeyFrame::TimeStamp.
     *
     * @note Duration is expressed in seconds after any ticks-per-second conversion during import.
     * @see Boom::Joint, Boom::KeyFrame, Boom::Animator
     */
    struct Animation
    {
        /** @brief Total clip length in seconds (post-import). */
        float Duration = 0.0f;

        /**
        * @brief Playback speed multiplier.
        *
        * 1.0 = authored speed, 0.5 = half speed (slow motion), 2.0 = double speed.
        * The effective sampling time is usually: @code t_effective = t * Speed; @endcode
        */
        float Speed = 1.0f;

        /** @brief Human-readable clip name (e.g., "Walk", "Idle", "Run"). */
        std::string Name;
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
         * @note TimeStamp is expected in seconds and should be monotonically non-decreasing per track.
         * @warning The identity quaternion is `glm::quat(1, 0, 0, 0)`; ensure your defaults/loads produce a valid unit quat.
         * @see Boom::Animator, Boom::Joint
         */
    struct KeyFrame
    {
        glm::vec3 Position = glm::vec3(0.0f);
        glm::quat Rotation = glm::vec3(0.0f);
        glm::vec3 Scale = glm::vec3(1.0f);
        float TimeStamp = 0.0f;
    };


    /**
    * @brief A node in the skeleton hierarchy (a.k.a. bone/joint).
    *
    * Each joint may have:
    * - Children that form the tree/graph of the skeleton.
    * - A set of animation @ref KeyFrame samples (its track) for the currently loaded clip.
    * - An inverse bind (offset) matrix used to transform skinned vertices from model space
    *   into joint space at bind pose for skinning.
    *
    * During skinning on CPU/GPU, the typical pipeline is:
    * 1) Evaluate each joint's local TRS at time *t* via interpolation of its @ref Keys.
    * 2) Accumulate to a global (model-space) matrix along the parent chain.
    * 3) Multiply by @ref Offset (inverse bind) to get the final skinning matrix for that joint.
    *
    * @note @ref Index maps the joint to a slot in the shader's joint matrix array (e.g., `u_Joints[Index]`).
    * @see Boom::Animation, Boom::KeyFrame
    */
    struct Joint
    {
        std::vector<Joint> Children{};
        std::vector<KeyFrame> Keys{};
        std::string Name{};
        glm::mat4 Offset{}; // <- inverse transform mtx
        int32_t Index{};
    };
}