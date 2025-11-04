// GridChaseAI.h
#pragma once
#include "GridAStar.h"
#include "GridReverseDjik.h"
#include "ECS/ECS.hpp"

namespace Boom {

    struct VelocityComponent { glm::vec3 vel{ 0 }; };

    struct VisionComponent {
        float radius = 12.f, fovDeg = 90.f, loseAfter = 1.f;
        bool hasLOS = false; float lastSeenTimer = 0.f;
        glm::vec3 lastSeenPos{ 0 };
    };

    enum class GridAlgo : uint8_t { AStar, FlowField };

    struct GridAgentComponent {
        GridAlgo algo = GridAlgo::AStar;
        float speed = 4.0f;
        float waypointEps = 0.1f;
        float replanCooldown = 0.25f; float replanTimer = 0.f;
        glm::vec3 lastPlannedGoal{ std::numeric_limits<float>::infinity() };
        std::vector<glm::vec3> waypoints; size_t wpIndex = 0;
    };

    // cheap FOV (XZ)
    inline bool InFOV_XZ(const glm::vec3& pos, const glm::vec3& fwd, const glm::vec3& tgt, float fovDeg)
    {
        glm::vec2 d{ tgt.x - pos.x, tgt.z - pos.z };
        float L2 = d.x * d.x + d.y * d.y; if (L2 < 1e-8f) return true;
        glm::vec2 nf{ fwd.x,fwd.z }; float n = std::sqrt(nf.x * nf.x + nf.y * nf.y); if (n < 1e-8f) return true;
        nf /= n; float dot = (nf.x * d.x + nf.y * d.y) / std::sqrt(L2);
        return dot >= std::cos(glm::radians(fovDeg * 0.5f));
    }

    // Bresenham-based LOS in world space (via grid)
    inline bool HasGridLOS(const Grid& grid, const glm::vec3& from, const glm::vec3& to)
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

    inline void UpdateGridChase(entt::registry& reg, entt::entity player, const GridContext& ctx, float dt)
    {
        if (!ctx.grid) return;
        const Grid& grid = *ctx.grid;
        const auto& tPl = reg.get<TransformComponent>(player);

        auto view = reg.view<TransformComponent, VelocityComponent, VisionComponent, GridAgentComponent>();
        view.each([&](auto,
            TransformComponent& tAI,
            VelocityComponent& vel,
            VisionComponent& vis,
            GridAgentComponent& agent)
            {
                // 1) Perception (range + FOV + LOS via grid)
                glm::vec3 playerPos = tPl.position;
                bool inRange = glm::length2(playerPos - tAI.position) <= vis.radius * vis.radius;
                bool inFov = inRange ? InFOV_XZ(tAI.position, tAI.forward, playerPos, vis.fovDeg) : false;
                bool los = inRange && inFov && HasGridLOS(grid, tAI.position, playerPos);

                vis.hasLOS = los;
                if (los) { vis.lastSeenPos = playerPos; vis.lastSeenTimer = 0.f; }
                else      vis.lastSeenTimer += dt;

                agent.replanTimer -= dt;

                if (los) {
                    // Direct chase (no path)
                    agent.waypoints.clear(); agent.wpIndex = 0; agent.lastPlannedGoal = playerPos;
                    vel.vel = Seek(tAI.position, playerPos, agent.speed);
                    if (glm::length2(vel.vel) > 1e-6f) tAI.forward = glm::normalize(vel.vel);
                    return;
                }

                // 2) No LOS ? use grid pathing toward last-seen cell
                if (agent.algo == GridAlgo::AStar) {
                    bool needReplan = (agent.replanTimer <= 0.f) ||
                        (glm::length2(agent.lastPlannedGoal - vis.lastSeenPos) > 0.5f * 0.5f);
                    if (needReplan) {
                        auto s = grid.worldToCell(tAI.position);
                        auto g = grid.worldToCell(vis.lastSeenPos);
                        auto path = AStarGrid(grid, s, g, ctx.agentY);
                        agent.waypoints = path.ok ? path.waypoints : std::vector<glm::vec3>{};
                        agent.wpIndex = 0;
                        agent.lastPlannedGoal = vis.lastSeenPos;
                        agent.replanTimer = agent.replanCooldown;
                    }

                    glm::vec3 target = vis.lastSeenPos;
                    if (agent.wpIndex < agent.waypoints.size()) {
                        target = agent.waypoints[agent.wpIndex];
                        if (glm::length2(target - tAI.position) <= agent.waypointEps * agent.waypointEps) {
                            ++agent.wpIndex;
                            if (agent.wpIndex < agent.waypoints.size())
                                target = agent.waypoints[agent.wpIndex];
                            else
                                target = vis.lastSeenPos;
                        }
                    }
                    vel.vel = Seek(tAI.position, target, agent.speed);

                }
                else { // GridAlgo::FlowField
                    if (!ctx.flow) return;

                    // Step toward neighbor with lowest distance
                    glm::ivec2 cur = grid.worldToCell(tAI.position);
                    glm::ivec2 nxt = ctx.flow->bestNeighbor(grid, cur);
                    glm::vec3 target = (nxt == cur) ? vis.lastSeenPos : grid.cellToWorld(nxt.x, nxt.y, ctx.agentY);
                    vel.vel = Seek(tAI.position, target, agent.speed * 0.95f);
                }

                if (glm::length2(vel.vel) > 1e-6f) tAI.forward = glm::normalize(vel.vel);
            });
    }

} // namespace Boom
