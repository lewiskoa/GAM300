#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include "ECS/ECS.hpp"

namespace Boom {

  
    struct NavAgentComponent {
        glm::vec3 target{ 0.f };
        std::vector<glm::vec3> path; // straight path
        int   waypoint = 0;
        float speed = 2.5f;  // m/s
        float arrive = 0.15f; // meters
        bool  active = true;
        bool  dirty = false; // set true when target changes

		entt::entity follow = entt::null; //this is player entity to follow
        float repathCooldown = 0.25f;     // seconds between path rebuilds
        float retargetDist = 0.5f;      // re-path if player moved this far
        float repathTimer = 0.f;       // internal timer
    };

    class DetourNavSystem; // fwd

    class NavAgentSystem {
    public:
        void requestPath(entt::registry& reg, entt::entity e, DetourNavSystem& nav);

        void update(entt::registry& reg, float dt, DetourNavSystem& nav)
        {
            auto view = reg.view<Transform3D, NavAgentComponent>();
            for (auto e : view) {
                auto& tr = view.get<Transform3D>(e);
                auto& ag = view.get<NavAgentComponent>(e);
                if (!ag.active) continue;

                // FOLLOW MODE: keep target synced to the followed entity (e.g., player)
                if (ag.follow != entt::null && reg.valid(ag.follow) && reg.all_of<Transform3D>(ag.follow)) {
                    ag.repathTimer -= dt;
                    const glm::vec3 desired = reg.get<Transform3D>(ag.follow).translate;

                    // Only re-path if enough time passed AND target moved meaningfully.
                    if (ag.repathTimer <= 0.f &&
                        glm::distance2(desired, ag.target) > ag.retargetDist * ag.retargetDist) {
                        ag.target = desired;
                        ag.dirty = true;
                        ag.repathTimer = ag.repathCooldown;
                    }
                }

                if (ag.dirty) {
                    requestPath(reg, e, nav); // builds straight path into ag.path
                }

                // Move along path (unchanged)
                if (ag.path.empty() || ag.waypoint >= (int)ag.path.size()) continue;

                const glm::vec3 pos = tr.translate;
                const glm::vec3 goal = ag.path[ag.waypoint];
                const glm::vec3 to = goal - pos;
                const float d = glm::length(to);

                if (d <= ag.arrive) {
                    ++ag.waypoint;
                    if (ag.waypoint >= (int)ag.path.size()) ag.path.clear();
                    continue;
                }

                const glm::vec3 dir = (d > 0.f) ? (to / d) : glm::vec3(0);
                tr.translate += dir * ag.speed * dt;  // kinematic move (swap in physics if needed)
            }
        }
    };

} // namespace Boom

