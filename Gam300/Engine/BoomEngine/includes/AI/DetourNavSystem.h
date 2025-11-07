#pragma once

#include "Core.h"

struct dtNavMesh;
struct dtNavMeshQuery;
struct dtQueryFilter;
using dtPolyRef = unsigned long long;  // Detour typedef

namespace Boom {

    // Poly flags (mirror the RecastDemo defaults; adjust for your project)
    constexpr unsigned short POLYFLAGS_WALK = 0x01;
  
    constexpr unsigned short POLYFLAGS_DOOR = 0x04;
    constexpr unsigned short POLYFLAGS_JUMP = 0x08; // optional
    constexpr unsigned short POLYFLAGS_DISABLED = 0x10;
    constexpr unsigned short POLYFLAGS_ALL = 0xffff;

    struct PathResult {
        bool                                success{ false };
        std::vector<glm::vec3>              points;       // straight-path world positions
        std::vector<dtPolyRef>              polys;        // poly corridor (useful for debugging)
    };

    class DetourNavSystem {
    public:
        DetourNavSystem();
        ~DetourNavSystem();

        // Initialize from a Detour binary blob (created by dtCreateNavMeshData).
        // We'll copy the data into Detour-owned memory so the caller can free the input buffer.
        bool initFromDetourData(const void* data, int sizeBytes);

        // Convenience: read a .bin produced by your editor/export step.
        bool initFromFile(const std::string& filepath);

        void shutdown();

        // Basic path query from 'start' to 'end'.
        PathResult findPath(const glm::vec3& start, const glm::vec3& end,
            const dtQueryFilter* customFilter = nullptr,
            const float* searchExtents = nullptr) const;

        // Raycast along the navmesh surface (useful for LOS checks).
        bool raycastSurface(const glm::vec3& start, const glm::vec3& end,
            glm::vec3& hitPos, float& hitT,
            const dtQueryFilter* customFilter = nullptr,
            const float* searchExtents = nullptr) const;

        // Utility: find a nearest navigable point for a given position.
        bool nearestPoint(const glm::vec3& in, glm::vec3& out,
            dtPolyRef& outRef,
            const dtQueryFilter* customFilter = nullptr,
            const float* searchExtents = nullptr) const;

        // Accessors
        BOOM_INLINE dtNavMesh* mesh()  const { return m_mesh; }
        BOOM_INLINE dtNavMeshQuery* query() const { return m_query; }
        BOOM_INLINE const dtQueryFilter& defaultFilter() const { return m_filter; }

        // Configure default filter include/exclude flags.
        void setFilter(unsigned short include = (POLYFLAGS_WALK |  POLYFLAGS_DOOR),
            unsigned short exclude = POLYFLAGS_DISABLED);

        // Set search extents used by nearestPoint() and pathfinding when none are provided.
        // ext = half-extents AABB in (x,y,z); typical = (2,4,2) for human-sized agents.
        void setDefaultSearchExtents(const float ext[3]);

    private:
        dtNavMesh* m_mesh{ nullptr };
        dtNavMeshQuery* m_query{ nullptr };
        dtQueryFilter    m_filter{};
        float            m_extents[3]{ 2.f, 4.f, 2.f };
    };

} // namespace Boom