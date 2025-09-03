#pragma once
#include "Helper.h"

namespace Boom
{
    struct Animation
    {
        float Duration = 0.0f;
        float Speed = 1.0f;
        std::string Name;
    };

    // keyframes: Animators define keyframes to
    //    represent significant poses or positions in the
    //    animation timeline.Each keyframe contains
    //    information about the position, rotation, and scale
    //    of each bone in the skeleton at that particular
    //    moment
    struct KeyFrame
    {
        glm::vec3 Position = glm::vec3(0.0f);
        glm::quat Rotation = glm::vec3(0.0f);
        glm::vec3 Scale = glm::vec3(1.0f);
        float TimeStamp = 0.0f;
    };

    struct Joint
    {
        std::vector<Joint> Children;
        std::vector<KeyFrame> Keys;
        std::string Name;
        glm::mat4 Offset; // <- inverse transform mtx
        int32_t Index;
    };
}