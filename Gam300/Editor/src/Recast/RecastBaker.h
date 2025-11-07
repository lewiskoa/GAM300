#pragma once
#include "Core.h"


namespace EditorUI {


	struct RecastBakeConfig {
		// World units (meters)
		float cellSize = 0.3f; // rcConfig::cs
		float cellHeight = 0.2f; // rcConfig::ch
		float agentHeight = 2.0f;
		float agentRadius = 0.6f;
		float agentMaxClimb = 0.9f;
		float agentMaxSlope = 45.0f; // degrees


		// Voxelization & regioning
		int regionMinArea = 8; // in voxels (after divide by 2,2)
		int regionMergeArea = 20; // in voxels
		float edgeMaxLen = 12.0f; // in meters, scaled by cellSize internally
		float edgeMaxError = 1.3f;
		int vertsPerPoly = 6; // DT_VERTS_PER_POLYGON, up to 6 typical
		float detailSampleDist = 6.0f; // 0=disabled, set ~6*cs
		float detailSampleMaxError = 1.0f;
	};


	// Simple triangle soup input. Provide world-space geometry you want walkable.
	struct RecastBakeInput {
		std::vector<float> verts; // [x0,y0,z0, x1,y1,z1, ...]
		std::vector<int> tris; // [i0,i1,i2, ...] indexing into verts/3
	};


	// Bake a solo (non-tiled) navmesh and write to Detour .bin.
	// Returns true on success, false on failure (error contains a user-readable message).
	bool RecastBakeToFile(const RecastBakeInput& in,
		const RecastBakeConfig& cfg,
		const std::string& outPath,
		std::string* error = nullptr);


} // namespace Boom
