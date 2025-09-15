#include "Scripting/ScriptRuntime.h"

#include <cassert>

EngineHooks ScriptRuntime::s_hooks{};
std::unordered_map<std::string, ScriptRuntime::ScriptType> ScriptRuntime::s_types{};
std::unordered_map<uint64_t, ScriptRuntime::ScriptType>    ScriptRuntime::s_instances{};
std::unordered_map<uint64_t, Entity>                       ScriptRuntime::s_instanceEntity{};
std::atomic_uint64_t                                       ScriptRuntime::s_nextId{ 1 };
std::mutex                                                 ScriptRuntime::s_mutex;

void ScriptRuntime::Initialize(const EngineHooks& hooks)
{
    s_hooks = hooks;
    // Light sanity checks—assert in debug, fail-soft in release.
    assert(s_hooks.Log && "EngineHooks.Log must be set");
    assert(s_hooks.CreateEntity && "EngineHooks.CreateEntity must be set");
    assert(s_hooks.SetPosition && "EngineHooks.SetPosition must be set");
    assert(s_hooks.GetPosition && "EngineHooks.GetPosition must be set");
}

void ScriptRuntime::Shutdown()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    // Destroy all still-live instances (call back into C# so it can OnDestroy)
    for (auto& [id, vtbl] : s_instances) {
        if (vtbl.d) vtbl.d(id);
    }
    s_instances.clear();
    s_instanceEntity.clear();
    s_types.clear();
}

void ScriptRuntime::RegisterType(const std::string& typeName,
    CreateFn c, UpdateFn u, DestroyFn d)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    s_types[typeName] = ScriptType{ c, u, d };
    if (s_hooks.Log) s_hooks.Log((std::string("[ScriptRuntime] Registered ") + typeName).c_str());
}

uint64_t ScriptRuntime::NextId()
{
    return s_nextId.fetch_add(1, std::memory_order_relaxed);
}

uint64_t ScriptRuntime::CreateInstance(const std::string& typeName, Entity e)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    auto it = s_types.find(typeName);
    if (it == s_types.end()) {
        if (s_hooks.Log) s_hooks.Log((std::string("[ScriptRuntime] Unknown type: ") + typeName).c_str());
        return 0;
    }

    const auto id = NextId();
    s_instances[id] = it->second;
    s_instanceEntity[id] = e;

    if (it->second.c) {
        // Call into C# OnCreate via CreateFn(entity, id)
        it->second.c(e, id);
    }
    return id;
}

void ScriptRuntime::DestroyInstance(uint64_t id)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    if (auto it = s_instances.find(id); it != s_instances.end()) {
        if (it->second.d) it->second.d(id); // C# OnDestroy
        s_instances.erase(it);
    }
    s_instanceEntity.erase(id);
}

void ScriptRuntime::UpdateInstance(uint64_t id, float dt)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    if (auto it = s_instances.find(id); it != s_instances.end()) {
        if (it->second.u) it->second.u(id, dt); // C# OnUpdate
    }
}

void ScriptRuntime::UpdateAll(float dt)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    for (auto& [id, vtbl] : s_instances) {
        if (vtbl.u) vtbl.u(id, dt);
    }
}
