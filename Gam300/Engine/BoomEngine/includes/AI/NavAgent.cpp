#include "Core.h"
#include "DetourNavSystem.h"
#include "NavAgent.h"

namespace Boom {

    void NavAgentSystem::requestPath(entt::registry& reg, entt::entity e, DetourNavSystem& nav)
    {
        auto& tr = reg.get<TransformComponent>(e);
        auto& ag = reg.get<NavAgentComponent>(e);

        const glm::vec3 start = tr.transform.translate;
        auto res = nav.findPath(start, ag.target);

        ag.path = std::move(res.points);
        ag.waypoint = 0;
        ag.dirty = false;
    }

} // namespace Boom