// AI/Actions.h
#pragma once
#include "NavAgent.h"
#include "BehaviourTree.h"
#include "AIComponent.h"
#include "ECS/ECS.hpp"
#include "glm/glm.hpp"

namespace Boom {

    struct SeePlayerCond : BTNode {
        BTState tick(entt::registry& reg, entt::entity e, float) override {
            auto& tr = reg.get<TransformComponent>(e);
            auto& ai = reg.get<AIComponent>(e);

            // Resolve/correct cache only if needed
            if (ai.player == entt::null || !reg.valid(ai.player) || !reg.all_of<TransformComponent>(ai.player)) {
                ai.player = FindEntityByName(reg, ai.playerName); // <— your function
                if (ai.player == entt::null || !reg.all_of<TransformComponent>(ai.player))
                    return BTState::Failure;
            }

            const glm::vec3 me = tr.transform.translate;
            const glm::vec3 pp = reg.get<TransformComponent>(ai.player).transform.translate;
            const float d2 = glm::distance2(me, pp);
            return (d2 <= ai.detectRadius * ai.detectRadius) ? BTState::Success : BTState::Failure;
        }
    };

    // When chasing, stop if we lost the player far enough
    struct StillChasingCond : BTNode {
        BTState tick(entt::registry& reg, entt::entity e, float) override {
            auto& tr = reg.get<TransformComponent>(e);
            auto& ai = reg.get<AIComponent>(e);

            if (ai.player == entt::null || !reg.valid(ai.player) || !reg.all_of<TransformComponent>(ai.player))
                return BTState::Failure;

            const glm::vec3 me = tr.transform.translate;
            const glm::vec3 pp = reg.get<TransformComponent>(ai.player).transform.translate;
            const float d2 = glm::distance2(me, pp);
            return (d2 <= ai.loseRadius * ai.loseRadius) ? BTState::Success : BTState::Failure;
        }
    };

    struct IdleAction : BTNode {
        BTState tick(entt::registry& reg, entt::entity e, float dt) override {
            auto& ai = reg.get<AIComponent>(e);
            ai.idleTimer -= dt;
            if (ai.idleTimer <= 0.f) return BTState::Success;
            return BTState::Running;
        }
    };

    // Drives NavAgent to walk patrol loop. Success when it *issued* movement; Running while moving.
    struct PatrolAction : BTNode {
        BTState tick(entt::registry& reg, entt::entity e, float) override {
            if (!reg.all_of<NavAgentComponent, TransformComponent, AIComponent>(e))
                return BTState::Failure;
            auto& ai = reg.get<AIComponent>(e);
            auto& ag = reg.get<NavAgentComponent>(e);

            if (ai.patrolPoints.empty()) return BTState::Failure;

            const glm::vec3 goal = ai.patrolPoints[ai.patrolIndex];
            if (ag.target != goal) { ag.follow = entt::null; ag.target = goal; ag.dirty = true; }

            // Have we reached this patrol point? (use arrive threshold from NavAgent)
            auto& tr = reg.get<TransformComponent>(e);
            if (glm::distance(tr.transform.translate, goal) <= ag.arrive) {
                // advance & idle
                ai.patrolIndex = (ai.patrolIndex + 1) % static_cast<int>(ai.patrolPoints.size());
                ai.idleTimer = ai.idleWait;
                return BTState::Success; // let an IdleAction run next frame
            }
            return BTState::Running;
        }
    };

    // Make agent chase the player by wiring NavAgent.follow
    struct SeekPlayerAction : BTNode {
        BTState tick(entt::registry& reg, entt::entity e, float) override {
            if (!reg.all_of<NavAgentComponent, AIComponent>(e)) return BTState::Failure;
            auto& ai = reg.get<AIComponent>(e);
            auto& ag = reg.get<NavAgentComponent>(e);

            if (ai.player == entt::null || !reg.valid(ai.player)) return BTState::Failure;
            if (ag.follow != ai.player) { ag.follow = ai.player; ag.dirty = true; }
            return BTState::Running;
        }
    };

} // namespace Boom
