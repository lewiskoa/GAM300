#pragma once
#include "Core.h"
#include <string>
#include <vector>

namespace Boom {

    class BOOM_API Profiler {
    public:
        struct ProfileData { float totalTime = 0.f; float lastFrameTime = 0.f; };
        struct Row { std::string name; ProfileData data; };

        Profiler() : p_(new Impl()) {}
       ~Profiler() { delete p_; p_ = nullptr; }
        Profiler(const Profiler&) = delete; Profiler& operator=(const Profiler&) = delete;
        Profiler(Profiler&& other) noexcept : p_(other.p_) {
            other.p_ = nullptr;
        }

        Profiler& operator=(Profiler&& other) noexcept {
            if (this != &other) {
                delete p_;
                p_ = other.p_;
                other.p_ = nullptr;
            }
            return *this;
        }

             // optional per-frame reset
        void Start(const std::string& name) {
            p_->start[name] = Impl::Clock::now();
        }
        void End(const std::string& name) {
            auto it = p_->start.find(name);
            if (it == p_->start.end()) return; // guard against mismatched Start/End
            const float ms = std::chrono::duration<float, std::milli>(Impl::Clock::now() - it->second).count();
            p_->curMs[name] += ms;  
            p_->curTotalMs += ms;
        }

        // Give Editor a copy it can render
        std::vector<Row> Snapshot() const {
            std::vector<Row> out;
            out.reserve(p_->profiles.size());
            for (const auto& kv : p_->profiles)
                out.push_back(Row{ kv.first, kv.second });

            // sort by last frame time (descending) so hottest is first
            std::sort(out.begin(), out.end(),
                [](const Row& a, const Row& b) {
                    constexpr float kEps = 10.f; // 0.05 ms threshold to avoid swaps
                    float da = a.data.lastFrameTime, db = b.data.lastFrameTime;
                    if (fabsf(da - db) > kEps) return da > db;   // primary: larger first
                    return a.name < b.name;                      // tie-breaker: by name
                });
            return out;
        }
        float SnapshotTotalMs() const {
            return p_->totalLastMs;
        }

        void BeginFrame() {
            p_->start.clear();        // no active timers
            p_->curMs.clear();        // reset working buckets
            p_->curTotalMs = 0.f;     // reset working total
        }

        void EndFrame() {
            p_->totalLastMs = p_->curTotalMs;
            for (const auto& kv : p_->curMs) {
                auto& pd = p_->profiles[kv.first];
                pd.lastFrameTime = kv.second;      // this frame total for that label
                pd.totalTime += kv.second;      // lifetime sum
            }
        }
    private:
        struct Impl {
           
                using Clock = std::chrono::high_resolution_clock;

                std::unordered_map<std::string, ProfileData> profiles;     // per-label stats
                std::unordered_map<std::string, Clock::time_point> start;  // active timers
                float totalLastMs = 0.0f;                                   // sum of lastFrameTime
                std::unordered_map<std::string, float> curMs;
                float curTotalMs = 0.f;

               
        
        
        };
        // defined in .cpp
        Impl* p_;         // raw pointer avoids C4251
    };

} // namespace Boom
