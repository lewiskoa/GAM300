#pragma once
// GridFlowField.h
#pragma once
#include "Grid.h"

namespace Boom {

    struct FlowField {
        int goalX = -1, goalY = -1;
        std::vector<float> dist; // size = W*H; INF if unreachable

        void compute(const Grid& g, glm::ivec2 goal)
        {
            goalX = goal.x; goalY = goal.y;
            const int N = g.W * g.H;
            dist.assign(N, std::numeric_limits<float>::infinity());

            struct Q { float d; int x, y; };
            struct C { bool operator()(const Q& a, const Q& b) const { return a.d > b.d; } };
            std::priority_queue<Q, std::vector<Q>, C> pq;

            auto K = [&](int x, int y) { return y * g.W + x; };
            if (!g.inBounds(goal.x, goal.y) || !g.passable(goal.x, goal.y)) return;

            dist[K(goal.x, goal.y)] = 0.f;
            pq.push({ 0.f, goal.x, goal.y });

            std::array<glm::ivec2, 8> nb; int nnb = 0;
            while (!pq.empty()) {
                auto cur = pq.top(); pq.pop();
                if (cur.d != dist[K(cur.x, cur.y)]) continue;
                gatherNeighbors(g, cur.x, cur.y, nb, nnb);
                for (int i = 0; i < nnb; ++i) {
                    auto nn = nb[i];
                    bool diag = (nn.x != cur.x && nn.y != cur.y);
                    float step = (diag ? 1.41421356f : 1.0f) * g.stepCost(nn.x, nn.y);
                    float cand = cur.d + step;
                    int k = K(nn.x, nn.y);
                    if (cand < dist[k]) { dist[k] = cand; pq.push({ cand, nn.x, nn.y }); }
                }
            }
        }

        // Returns next step (cell) that most decreases distance; if none, returns cur.
        glm::ivec2 bestNeighbor(const Grid& g, glm::ivec2 cur) const
        {
            auto K = [&](int x, int y) { return y * g.W + x; };
            if (dist.empty() || !g.inBounds(cur.x, cur.y)) return cur;
            float best = dist[K(cur.x, cur.y)];
            glm::ivec2 pick = cur;
            std::array<glm::ivec2, 8> nb; int nnb = 0; gatherNeighbors(g, cur.x, cur.y, nb, nnb);
            for (int i = 0; i < nnb; ++i) {
                auto nn = nb[i];
                float d = dist[K(nn.x, nn.y)];
                if (d < best) { best = d; pick = nn; }
            }
            return pick;
        }
    };

} // namespace Boom
