#pragma once

#pragma once
#include "Grid.h"


namespace Boom {

    struct GridPath {
        bool ok = false;
        std::vector<glm::ivec2> cells;       // cell coordinates
        std::vector<glm::vec3>  waypoints;   // world-space centers (XZ)
    };

    inline float h_octile(glm::ivec2 a, glm::ivec2 b) {
        int dx = std::abs(a.x - b.x), dy = std::abs(a.y - b.y);
        int m = std::min(dx, dy), M = std::max(dx, dy);
        return m * 1.41421356f + (M - m) * 1.0f;
    }

    inline GridPath AStarGrid(const Grid& g, glm::ivec2 s, glm::ivec2 t, float yWorld = 0.f)
    {
        GridPath out;
        if (!g.inBounds(s.x, s.y) || !g.inBounds(t.x, t.y) || !g.passable(t.x, t.y)) return out;

        struct Q { float f, g; int x, y; };
        struct C { bool operator()(const Q& a, const Q& b) const { return a.f > b.f; } };
        std::priority_queue<Q, std::vector<Q>, C> open;

        const int N = g.W * g.H;
        std::vector<float> gCost(N, std::numeric_limits<float>::infinity());
        std::vector<int>   parent(N, -1);
        auto K = [&](int x, int y) { return y * g.W + x; };

        gCost[K(s.x, s.y)] = 0.f;
        open.push({ h_octile(s,t), 0.f, s.x, s.y });

        std::array<glm::ivec2, 8> nb; int nnb = 0;
        while (!open.empty()) {
            Q cur = open.top(); open.pop();
            if (cur.x == t.x && cur.y == t.y) break;

            gatherNeighbors(g, cur.x, cur.y, nb, nnb);
            for (int i = 0; i < nnb; ++i) {
                auto nn = nb[i];
                const bool diag = (nn.x != cur.x && nn.y != cur.y);
                float step = diag ? 1.41421356f : 1.0f;
                float cand = gCost[K(cur.x, cur.y)] + step * g.stepCost(nn.x, nn.y);
                int k = K(nn.x, nn.y);
                if (cand < gCost[k]) {
                    gCost[k] = cand; parent[k] = K(cur.x, cur.y);
                    float f = cand + h_octile(nn, t);
                    open.push({ f, cand, nn.x, nn.y });
                }
            }
        }
        if (parent[K(t.x, t.y)] == -1 && !(s == t)) return out;

        // Reconstruct cells
        std::vector<glm::ivec2> cells;
        for (int v = K(t.x, t.y); v != -1; v = parent[v]) {
            cells.push_back({ v % g.W, v / g.W });
            if (v == K(s.x, s.y)) break;
        }
        std::reverse(cells.begin(), cells.end());

        // Optional straightening using grid LOS (skip intermediate waypoints)
        std::vector<glm::ivec2> simplified;
        if (!cells.empty()) {
            size_t i = 0; simplified.push_back(cells[0]);
            while (i + 1 < cells.size()) {
                size_t j = cells.size() - 1;
                for (; j > i + 1; --j) {
                    if (gridLineOfSightClear(g, cells[i], cells[j])) { break; }
                }
                simplified.push_back(cells[j]);
                i = j;
            }
        }
        else simplified = cells;

        // World waypoints
        std::vector<glm::vec3> wps; wps.reserve(simplified.size());
        for (auto c : simplified) wps.push_back(g.cellToWorld(c.x, c.y, yWorld));

        out.ok = !wps.empty();
        out.cells = std::move(simplified);
        out.waypoints = std::move(wps);
        return out;
    }

} // namespace Boom
