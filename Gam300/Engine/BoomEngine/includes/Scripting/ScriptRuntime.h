#pragma once
#include "GlobalConstants.h"
#include <string>
#include <unordered_map>
#include <atomic>
#include <mutex>

// Engine functions you wire up from the engine side
struct EngineHooks {
    void (*Log)(const char* msg) = nullptr;

    // ECS via integer handles; keep your engine Entity private
    Boom::EntityId(*CreateEntity)() = nullptr;
    void           (*DestroyEntity)(Boom::EntityId) = nullptr;

    // Transform
    void         (*SetPosition)(Boom::EntityId, const Boom::Vec3&) = nullptr;
    Boom::Vec3(*GetPosition)(Boom::EntityId) = nullptr;
};

class ScriptRuntime {
public:
    using CreateFn = void(*)(Boom::EntityId, std::uint64_t);
    using UpdateFn = void(*)(std::uint64_t, float);
    using DestroyFn = void(*)(std::uint64_t);

    struct ScriptType {
        CreateFn  c = nullptr;
        UpdateFn  u = nullptr;
        DestroyFn d = nullptr;
    };

    static void           Initialize(const EngineHooks& hooks);
    static void           Shutdown();

    static void           RegisterType(const std::string& typeName, CreateFn c, UpdateFn u, DestroyFn d);
    static std::uint64_t  CreateInstance(const std::string& typeName, Boom::EntityId e);
    static void           DestroyInstance(std::uint64_t id);
    static void           UpdateInstance(std::uint64_t id, float dt);
    static void           UpdateAll(float dt);

    static EngineHooks& Hooks() { return s_hooks; }

private:
    static std::uint64_t  NextId();

    static EngineHooks s_hooks;

    static std::unordered_map<std::string, ScriptType>      s_types;
    static std::unordered_map<std::uint64_t, ScriptType>    s_instances;
    static std::unordered_map<std::uint64_t, Boom::EntityId> s_instanceEntity;

    static std::atomic_uint64_t s_nextId;
    static std::mutex           s_mutex;
};
