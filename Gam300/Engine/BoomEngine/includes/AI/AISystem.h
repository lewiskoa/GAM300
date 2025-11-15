#pragma once
#include "AIComponent.h"
#include "BehaviourTreeActions.h"
#include "Application/Interface.h"
#include "ECS/ECS.hpp"
namespace Boom {
    BOOM_INLINE void ApplyModeSideEffects(entt::registry& reg, entt::entity e, AIComponent& ai)
    {
        auto& ag = reg.get_or_emplace<NavAgentComponent>(e);
        switch (ai.mode)
        {
        case AIComponent::AIMode::Idle:
            ag.follow = entt::null;
            ag.active = false;
            ag.path.clear();
            ag.waypoint = 0;
            ag.dirty = false;
            ag.repathTimer = 0.f;
            break;
        case AIComponent::AIMode::Patrol:
            ag.active = true;
            ag.follow = entt::null;
            ag.dirty = true;
            ag.repathTimer = 0.f;
            break;
        case AIComponent::AIMode::Seek:
            ag.active = true;
            if (ai.player != entt::null) {
                ag.follow = ai.player;
                ag.dirty = true;
                ag.repathTimer = 0.f;
            }
            break;
        case AIComponent::AIMode::Auto:
        default:
            ag.active = true;
            break;
        }
    }
    struct AISystem {
        // Call once when creating an enemy
        static void InitEnemy(entt::registry& reg, entt::entity e,
            const std::vector<glm::vec3>& patrolPts,
            float detect = 8.f, float lose = 12.f, float idle = 1.f)
        {
            auto& ai = reg.emplace_or_replace<AIComponent>(e);
            ai.patrolPoints = patrolPts;
            ai.detectRadius = detect;
            ai.loseRadius = lose;
            ai.idleWait = idle;
            ai.patrolIndex = 0;
            ai.idleTimer = 0.f;
            ai.root = BuildPatrolSeekTree(); // attach tree
        }

        // Run every frame BEFORE NavAgentSystem::update (so it can flag ag.dirty)
        void update(entt::registry& reg, float dt)
        {
            auto view = reg.view<AIComponent>();
            for (auto e : view) {
                auto& ai = view.get<AIComponent>(e);

                if (ai.mode != ai.lastMode || !ai.root) {
                    ApplyModeSideEffects(reg, e, ai);
                    ai.root = BuildTreeForMode(ai.mode);
                    ai.lastMode = ai.mode;
                }

                if (ai.root) ai.root->tick(reg, e, dt);
            }
        }
    };

} // namespace Boom