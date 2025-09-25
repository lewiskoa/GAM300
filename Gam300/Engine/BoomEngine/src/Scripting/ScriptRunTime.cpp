#include "Core.h"
#include "Scripting/ScriptRuntime.h"

#include <unordered_map>
#include <atomic>
#include <mutex>
#include <string>

namespace ScriptRuntime {

    // --------------------------
    // Hooks table (single TU)
    // --------------------------
    static EngineHooks g_hooks{};
    const EngineHooks& Hooks() noexcept { return g_hooks; }
    void SetHooks(const EngineHooks& h) { g_hooks = h; }
    void Initialize(const EngineHooks& h) { SetHooks(h); }

    // --------------------------
    // Registry of script types
    // --------------------------
    struct ScriptType {
        CreateCb  c{};
        UpdateCb  u{};
        DestroyCb d{};
    };

    static std::unordered_map<std::string, ScriptType>        s_types;
    static std::unordered_map<std::uint64_t, ScriptType>      s_instances;
    static std::unordered_map<std::uint64_t, Boom::EntityId>  s_instanceEntity;

    static std::atomic_uint64_t s_nextId{ 1 };
    static std::mutex           s_mutex;

    static std::uint64_t NextId() {
        return s_nextId.fetch_add(1, std::memory_order_relaxed);
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(s_mutex);
        for (auto& [id, vtbl] : s_instances) {
            if (vtbl.d) vtbl.d(id);
        }
        s_instances.clear();
        s_instanceEntity.clear();
        s_types.clear();
    }

    // --------------------------
    // Public API
    // --------------------------
    void RegisterType(const char* typeName, CreateCb c, UpdateCb u, DestroyCb d) {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_types[std::string(typeName)] = ScriptType{ c,u,d };
        if (g_hooks.Log) {
            std::string msg = "[ScriptRuntime] Registered ";
            msg += typeName;
            g_hooks.Log(msg.c_str());
        }
    }

    std::uint64_t CreateInstance(const char* typeName, Boom::EntityId e) {
        std::lock_guard<std::mutex> lock(s_mutex);

        auto it = s_types.find(typeName);
        if (it == s_types.end()) {
            if (g_hooks.Log) {
                std::string msg = "[ScriptRuntime] Unknown type: ";
                msg += typeName ? typeName : "(null)";
                g_hooks.Log(msg.c_str());
            }
            return 0;
        }

        const auto id = NextId();
        s_instances[id] = it->second;
        s_instanceEntity[id] = e;

        if (it->second.c) it->second.c(e, id);
        return id;
    }

    void DestroyInstance(std::uint64_t id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (auto it = s_instances.find(id); it != s_instances.end()) {
            if (it->second.d) it->second.d(id);
            s_instances.erase(it);
        }
        s_instanceEntity.erase(id);
    }

    void UpdateInstance(std::uint64_t id, float dt) {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (auto it = s_instances.find(id); it != s_instances.end()) {
            if (it->second.u) it->second.u(id, dt);
        }
    }

    void UpdateAll(float dt) {
        std::lock_guard<std::mutex> lock(s_mutex);
        for (auto& [id, vtbl] : s_instances) {
            if (vtbl.u) vtbl.u(id, dt);
        }
    }

    Boom::EntityId EntityOf(std::uint64_t id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (auto it = s_instanceEntity.find(id); it != s_instanceEntity.end())
            return it->second;
        return 0;
    }

} // namespace ScriptRuntime
