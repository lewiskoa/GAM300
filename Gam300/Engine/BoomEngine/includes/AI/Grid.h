#pragma once
// Grid.h
#pragma once
#include "Core.h"

namespace Boom {

    struct Grid {
        int   W = 0, H = 0;            // cells wide/high
        float tile = 1.0f;             // world units per cell
        glm::vec2 origin{ 0.f, 0.f };    // world-space of cell (0,0) center projected on XZ

        // 0 = free, >0 = blocked or extra cost (1 = wall, 5 = mud, etc.)
        std::vector<uint8_t> occ;      // size = W*H

        bool inBounds(int x, int y) const { return x >= 0 && y >= 0 && x < W && y < H; }
        int  key(int x, int y) const { return y * W + x; }
        bool passable(int x, int y) const { return inBounds(x, y) && occ[key(x, y)] == 0; }
        float stepCost(int x, int y) const { return inBounds(x, y) ? (occ[key(x, y)] ? 1e9f : 1.f) : 1e9f; }

        // Grid <-> world (XZ) mapping
        glm::ivec2 worldToCell(const glm::vec3& p) const {
            glm::vec2 local{ p.x - origin.x, p.z - origin.y };
            int x = (int)std::floor(local.x / tile + 0.5f);
            int y = (int)std::floor(local.y / tile + 0.5f);
            return { x, y };
        }
        glm::vec3 cellToWorld(int x, int y, float yWorld = 0.f) const {
            return { origin.x + x * tile, yWorld, origin.y + y * tile };
        }
    };

    // 8-neighborhood with corner-cut prevention (only allow diagonal if both sides free)
    BOOM_INLINE void gatherNeighbors(const Grid& g, int x, int y, std::array<glm::ivec2, 8>& out, int& nOut)
    {
        nOut = 0;
        auto add = [&](int nx, int ny) { if (g.passable(nx, ny)) out[nOut++] = { nx,ny }; };
        // orthogonals
        bool N = g.passable(x, y - 1), S = g.passable(x, y + 1), W = g.passable(x - 1, y), E = g.passable(x + 1, y);
        if (N) add(x, y - 1);
        if (S) add(x, y + 1);
        if (W) add(x - 1, y);
        if (E) add(x + 1, y);
        // diagonals (no corner cutting)
        if (N && W && g.passable(x - 1, y - 1)) add(x - 1, y - 1);
        if (N && E && g.passable(x + 1, y - 1)) add(x + 1, y - 1);
        if (S && W && g.passable(x - 1, y + 1)) add(x - 1, y + 1);
        if (S && E && g.passable(x + 1, y + 1)) add(x + 1, y + 1);
    }

    // Grid LOS (Bresenham on cells). Returns true if line of sight is CLEAR.
    BOOM_INLINE bool gridLineOfSightClear(const Grid& g, glm::ivec2 a, glm::ivec2 b)
    {
        int x0 = a.x, y0 = a.y, x1 = b.x, y1 = b.y;
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;

        while (true) {
            if (!g.inBounds(x0, y0) || !g.passable(x0, y0)) return false;
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
        return true;
    }

} // namespace Boom
