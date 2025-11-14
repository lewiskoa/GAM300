#pragma once

#include "BehaviourTree.h"
#include "Actions.h"
#include "ECS/ECS.hpp"
namespace Boom {

    inline std::unique_ptr<BTNode> BuildPatrolSeekTree()
    {
        using U = std::unique_ptr<BTNode>;
        // Root: Try chase branch; if it fails, do patrol branch
        return std::make_unique<Selector>(std::vector<U>{
            // If we can see player, keep chasing while we haven't lost them
            std::make_unique<Sequence>(std::vector<U>{
                std::make_unique<SeePlayerCond>(),
                    std::make_unique<SeekPlayerAction>(),
                    std::make_unique<StillChasingCond>() // keeps sequence running while valid
            }),
                // Patrol branch: go next waypoint, then idle a bit
                std::make_unique<Sequence>(std::vector<U>{
                std::make_unique<PatrolAction>(),
                    // Optional cooldown so it doesn't instantly switch points
                    std::make_unique<Cooldown>(std::make_unique<IdleAction>(), 0.0f)
            })
        });
    }

} // namespace Boom