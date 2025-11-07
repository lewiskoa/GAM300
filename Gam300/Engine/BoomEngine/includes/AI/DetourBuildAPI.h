#pragma once
#include "Core.h"
#include <cstdint>

namespace Boom {

    struct BoomNavCreateParams {
        // rcPolyMesh
        const unsigned short* verts = nullptr;      int vertCount = 0;
        const unsigned short* polys = nullptr;
        const unsigned char* polyAreas = nullptr;
        const unsigned short* polyFlags = nullptr;
        int                   polyCount = 0;
        int                   nvp = 0;

        // rcPolyMeshDetail
        const unsigned int* detailMeshes = nullptr;
        const float* detailVerts = nullptr; int detailVertsCount = 0;
        const unsigned char* detailTris = nullptr; int detailTriCount = 0;

        // Agent + voxel
        float walkableHeight = 0.f, walkableRadius = 0.f, walkableClimb = 0.f;
        float bmin[3]{}, bmax[3]{};
        float cs = 0.f, ch = 0.f;
        int   buildBvTree = 1;

        // Optional off-mesh links
        const float* offMeshConVerts = nullptr;
        const float* offMeshConRad = nullptr;
        const unsigned short* offMeshConFlags = nullptr;
        const unsigned char* offMeshConAreas = nullptr;
        const unsigned int* offMeshConUserID = nullptr;
        int                   offMeshConCount = 0;
    };

    // Engine builds Detour data and writes it to disk.
    BOOM_API bool BuildDetourBinaryToFile(const BoomNavCreateParams& p,
        const char* outPath);

} // namespace Boom
