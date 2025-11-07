#include "Core.h"
#include "DetourBuildAPI.h"

#include <DetourNavMeshBuilder.h>
#include <DetourAlloc.h>
#include <fstream>

namespace Boom {

    BOOM_API bool BuildDetourBinaryToFile(const BoomNavCreateParams& p,
        const char* outPath)
    {
        if (!outPath || !*outPath) return false;

        dtNavMeshCreateParams tmp{};
        // rcPolyMesh
        tmp.verts = p.verts; tmp.vertCount = p.vertCount;
        tmp.polys = p.polys; tmp.polyAreas = p.polyAreas; tmp.polyFlags = p.polyFlags;
        tmp.polyCount = p.polyCount; tmp.nvp = p.nvp;

        // rcPolyMeshDetail
        tmp.detailMeshes = p.detailMeshes;
        tmp.detailVerts = p.detailVerts; tmp.detailVertsCount = p.detailVertsCount;
        tmp.detailTris = p.detailTris;   tmp.detailTriCount = p.detailTriCount;

        // Agent + voxel
        tmp.walkableHeight = p.walkableHeight;
        tmp.walkableRadius = p.walkableRadius;
        tmp.walkableClimb = p.walkableClimb;
        tmp.bmin[0] = p.bmin[0]; tmp.bmin[1] = p.bmin[1]; tmp.bmin[2] = p.bmin[2];
        tmp.bmax[0] = p.bmax[0]; tmp.bmax[1] = p.bmax[1]; tmp.bmax[2] = p.bmax[2];
        tmp.cs = p.cs; tmp.ch = p.ch;
        tmp.buildBvTree = p.buildBvTree ? 1 : 0;

        // Off-mesh (optional)
        tmp.offMeshConVerts = p.offMeshConVerts;
        tmp.offMeshConRad = p.offMeshConRad;
        tmp.offMeshConFlags = p.offMeshConFlags;
        tmp.offMeshConAreas = p.offMeshConAreas;
        tmp.offMeshConUserID = p.offMeshConUserID;
        tmp.offMeshConCount = p.offMeshConCount;

        unsigned char* data = nullptr; int size = 0;
        if (!dtCreateNavMeshData(&tmp, &data, &size)) return false;

        std::ofstream f(outPath, std::ios::binary);
        if (!f) { dtFree(data); return false; }
        f.write(reinterpret_cast<const char*>(data), size);
        dtFree(data);
        return true;
    }

} // namespace Boom
