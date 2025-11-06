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
    };

    class DetourNavSystem; // fwd

    class NavAgentSystem {
    public:
        void requestPath(entt::registry& reg, entt::entity e, DetourNavSystem& nav);

        void update(entt::registry& reg, float dt, DetourNavSystem& nav)
        {
            auto view = reg.view<TransformComponent, NavAgentComponent>();
            for (auto e : view) {
                auto& tr = view.get<TransformComponent>(e);
                auto& ag = view.get<NavAgentComponent>(e);
                if (!ag.active) continue;

                if (ag.dirty) {
                    requestPath(reg, e, nav);
                }

                if (ag.path.empty() || ag.waypoint >= (int)ag.path.size()) continue;

                const glm::vec3 pos = tr.transform.translate;
                const glm::vec3 goal = ag.path[ag.waypoint];
                const glm::vec3 toDst = goal - pos;
                const float d = glm::length(toDst);

                if (d <= ag.arrive) {
                    ag.waypoint++;
                    if (ag.waypoint >= (int)ag.path.size()) {
                        ag.path.clear();
                    }
                    continue;
                }

                const glm::vec3 dir = (d > 0.f) ? (toDst / d) : glm::vec3(0);
                tr.transform.translate += dir * ag.speed * dt; // kinematic move
            }
        }
    };

} // namespace Boom

