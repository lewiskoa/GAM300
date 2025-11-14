#pragma once
#include "AIComponent.h"
#include "BehaviourTreeActions.h"
#include "Application/Interface.h"
#include "ECS/ECS.hpp"
namespace Boom {

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
        void update(entt::registry& reg, float dt) {
            auto view = reg.view<AIComponent>();
            for (auto e : view) {
                auto& ai = view.get<AIComponent>(e);
                if (!ai.root) ai.root = BuildPatrolSeekTree();
                ai.root->tick(reg, e, dt);
            }
        }
    };

} // namespace Boom