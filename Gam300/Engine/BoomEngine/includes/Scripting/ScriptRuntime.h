#pragma once

// Native glue between your ECS and the ScriptAPI C boundary.
// No C# includes here—pure C++.

#include "Core.h"

struct Vec3 { float x, y, z; };
using Entity = uint32_t;

// Delegates (must match ScriptAPI.h)
using CreateFn = void(*)(Entity entity, uint64_t instanceId);
using UpdateFn = void(*)(uint64_t instanceId, float dt);
using DestroyFn = void(*)(uint64_t instanceId);

// ---------------- EngineHooks ----------------
// You plug your real engine functions into this without exposing headers.
struct EngineHooks
{
    // Required
    void (*Log)(const char* msg) = nullptr;

    // ECS / Transform
    Entity(*CreateEntity)() = nullptr;
    void   (*SetPosition)(Entity, Vec3) = nullptr;
    Vec3(*GetPosition)(Entity) = nullptr;
};

// ---------------- ScriptRuntime ----------------
class ScriptRuntime
{
public:
    // Call once during engine/editor startup—before .NET registration begins.
    static void Initialize(const EngineHooks& hooks);

    // Optional: free all instances on shutdown
    static void Shutdown();

    // Registration from C# (one call per script type)
    static void RegisterType(const std::string& typeName,
        CreateFn c, UpdateFn u, DestroyFn d);

    // Instance management (used by Editor & scene loading)
    static uint64_t CreateInstance(const std::string& typeName, Entity e);
    static void     DestroyInstance(uint64_t id);

    // Per-frame update
    static void     UpdateInstance(uint64_t id, float dt);
    static void     UpdateAll(float dt);

    // These are used by ScriptAPI C exports:
    static inline const EngineHooks& Hooks() { return s_hooks; }

private:
    struct ScriptType { CreateFn c{}; UpdateFn u{}; DestroyFn d{}; };

    static EngineHooks                                    s_hooks;
    static std::unordered_map<std::string, ScriptType>    s_types;
    static std::unordered_map<uint64_t, ScriptType>       s_instances; // maps id -> vtable
    static std::unordered_map<uint64_t, Entity>           s_instanceEntity;
    static std::atomic_uint64_t                           s_nextId;
    static std::mutex                                     s_mutex;     // protects maps

    static uint64_t NextId();
};
