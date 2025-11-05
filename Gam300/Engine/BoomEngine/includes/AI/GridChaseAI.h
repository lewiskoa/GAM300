// GridChaseAI.h
#pragma once
#include "GridAStar.h"
#include "GridReverseDjik.h"
#include "ECS/ECS.hpp"

namespace Boom {
    BOOM_INLINE glm::vec3 ForwardFromYawDeg(float yawDeg) {
        float y = glm::radians(yawDeg);
        // OpenGL-ish: forward = -Z; XZ plane only
        return glm::normalize(glm::vec3(std::sin(y), 0.0f, -std::cos(y)));
    }
    struct VelocityAI {
        glm::vec3 vel{ 0.f };
        XPROPERTY_DEF("VelocityAI", VelocityAI,
            obj_member<"vel", &VelocityAI::vel>
        )
    };

    struct VisionAI {
        float     radius = 12.f;
        float     fovDeg = 360.f;
        float     loseAfter = 1.f;
        bool      hasLOS = false;
        float     lastSeenTimer = 0.f;
        glm::vec3 lastSeenPos{ 0.f,0.f,0.f };

        XPROPERTY_DEF("VisionAI", VisionAI,
            obj_member<"radius", &VisionAI::radius>,
            obj_member<"fovDeg", &VisionAI::fovDeg>,
            obj_member<"loseAfter", &VisionAI::loseAfter>,
            obj_member<"hasLOS", &VisionAI::hasLOS>,
            obj_member<"lastSeenTimer", &VisionAI::lastSeenTimer>,
            obj_member<"lastSeenPos", &VisionAI::lastSeenPos>
        )
    };

    struct ChaserAI {
        float speed = 10.f;
        XPROPERTY_DEF("ChaserAI", ChaserAI,
            obj_member<"speed", &ChaserAI::speed>
        )
    };

    struct DirectChaseAI {
        std::string  targetName = "player";
        entt::entity target = entt::null;   // runtime cache
        float        reacquireEvery = 0.25f;
        float        timer = 0.f;

        XPROPERTY_DEF("DirectChaseAI", DirectChaseAI,
            obj_member<"targetName", &DirectChaseAI::targetName>,
            obj_member<"reacquireEvery", &DirectChaseAI::reacquireEvery>
            // omit 'target' and 'timer' from UI (runtime)
        )
    };

    enum class GridAlgo : uint8_t { AStar, FlowField };

    struct GridAgentAI {
        GridAlgo algo = GridAlgo::AStar;
        float    speed = 4.0f;
        float    waypointEps = 0.1f;
        float    replanCooldown = 0.25f;
        float    replanTimer = 0.f;

        glm::vec3              lastPlannedGoal{ std::numeric_limits<float>::infinity() };
        std::vector<glm::vec3> waypoints;
        size_t                 wpIndex = 0;

        XPROPERTY_DEF("GridAgentAI", GridAgentAI,
            obj_member<"algo", &GridAgentAI::algo>,
            obj_member<"speed", &GridAgentAI::speed>,
            obj_member<"waypointEps", &GridAgentAI::waypointEps>,
            obj_member<"replanCooldown", &GridAgentAI::replanCooldown>
            // runtime: replanTimer/lastPlannedGoal/waypoints/wpIndex omitted
        )
    };

    // cheap FOV (XZ)
    BOOM_INLINE bool InFOV_XZ(const glm::vec3& pos, const glm::vec3& fwd, const glm::vec3& tgt, float fovDeg)
    {
        glm::vec2 d{ tgt.x - pos.x, tgt.z - pos.z };
        float L2 = d.x * d.x + d.y * d.y; if (L2 < 1e-8f) return true;
        glm::vec2 nf{ fwd.x,fwd.z }; float n = std::sqrt(nf.x * nf.x + nf.y * nf.y); if (n < 1e-8f) return true;
        nf /= n; float dot = (nf.x * d.x + nf.y * d.y) / std::sqrt(L2);
        return dot >= std::cos(glm::radians(fovDeg * 0.5f));
    }

    // Bresenham-based LOS in world space (via grid)
    BOOM_INLINE bool HasGridLOS(const Grid& grid, const glm::vec3& from, const glm::vec3& to)
    {
        return gridLineOfSightClear(grid, grid.worldToCell(from), grid.worldToCell(to));
    }

    inline glm::vec3 Seek(const glm::vec3& from, const glm::vec3& to, float speed)
    {
        glm::vec3 d = to - from;
        float L2 = glm::dot(d, d); if (L2 < 1e-6f) return glm::vec3(0);
        return (d / std::sqrt(L2)) * speed;
    }

    struct GridContext {
        const Grid* grid = nullptr;
        FlowField* flow = nullptr; // optional (only for GridAlgo::FlowField)
        float agentY = 0.f;         // y-level for waypoint centers
    };

    BOOM_INLINE void RunDirectChaseSystem(entt::registry& reg, float dt) {
        auto view = reg.view<DirectChaseComponent, VisionComponentAI, ChaserComponentAI,
            TransformComponent, RigidBodyComponent>();

        view.each([&](entt::entity e,
            DirectChaseComponent& aiC,
            VisionComponentAI& visC,
            ChaserComponentAI& chC,
            TransformComponent& tc,
            RigidBodyComponent& rb)
            {
                // Re-acquire target by name periodically or if missing
                auto& ai = aiC.directChaseAI;
                auto& vis = visC.visionAI;
                auto& chase = chC.chaserAI;
                ai.timer -= dt;
                if (ai.target == entt::null || ai.timer <= 0.f) {
                    ai.target = FindEntityByName(reg, ai.targetName);
                    ai.timer = ai.reacquireEvery;
                    if (ai.target == entt::null) return; // no target yet
                }

                // Read target position
                const auto& tPl = reg.get<TransformComponent>(ai.target).transform;
                const glm::vec3 playerPos = tPl.translate;

                // Perception (keep simple: 360° range check)
                const glm::vec3 myPos = tc.transform.translate;
                const bool inRange = glm::length2(playerPos - myPos) <= vis.radius * vis.radius;
                vis.hasLOS = inRange;                  // plug your LOS/FOV later if desired
                if (vis.hasLOS) vis.lastSeenPos = playerPos;

                // Choose destination
                const glm::vec3 goal = vis.hasLOS ? playerPos : vis.lastSeenPos;

                // Desired velocity
                const glm::vec3 desired = Seek(myPos, goal, chase.speed);

                // Apply (PhysX if present; else fallback to manual integration)
                if (rb.RigidBody.actor) {
                    if (auto* dyn = rb.RigidBody.actor->is<physx::PxRigidDynamic>()) {
                        dyn->setLinearVelocity(physx::PxVec3(desired.x, desired.y, desired.z));
                    }
                }
                else {
                    tc.transform.translate += desired * dt;
                }

                // Face movement direction on XZ (optional)
                if (glm::length2(desired) > 1e-6f) {
                    const glm::vec2 dir{ desired.x, desired.z };
                    const float yaw = std::atan2(dir.x, -dir.y);
                    tc.transform.rotate.y = glm::degrees(yaw);
                }
            });
    }

} // namespace Boom
