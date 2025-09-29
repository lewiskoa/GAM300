#pragma once
#include "Core.h"
#include <cstdint>
#include "GlobalConstants.h"   // for Boom::EntityId, Boom::Vec3 (adjust include if needed)

namespace ScriptRuntime {

    // ---- Script (managed) callbacks the runtime will call ----
    using CreateCb = void(*)(Boom::EntityId /*entity*/, std::uint64_t /*instanceId*/);
    using UpdateCb = void(*)(std::uint64_t /*instanceId*/, float /*dt*/);
    using DestroyCb = void(*)(std::uint64_t /*instanceId*/);

    // ---- Hooks: engine functions the runtime can call back into ----
    struct EngineHooks {
        void (*Log)(const char* msg) = nullptr;
        Boom::EntityId(*CreateEntity)() = nullptr;
        void (*DestroyEntity)(Boom::EntityId e) = nullptr;
        void (*SetPosition)(Boom::EntityId e, Boom::Vec3 p) = nullptr;
        Boom::Vec3(*GetPosition)(Boom::EntityId e) = nullptr;
        // add more as your engine surface grows (Get/SetRotation, FindByName, etc.)
    };

    // ---- Hooks accessors (declared here, defined once in .cpp) ----
    const EngineHooks& Hooks() noexcept;
    void SetHooks(const EngineHooks& h);

    // ---- Lifetime of the runtime ----
    void Initialize(const EngineHooks& h);   // convenience: calls SetHooks
    void Shutdown();

    // ---- Script type registry & instances ----
    void RegisterType(const char* typeName, CreateCb c, UpdateCb u, DestroyCb d);

    std::uint64_t CreateInstance(const char* typeName, Boom::EntityId e);
    void          DestroyInstance(std::uint64_t id);

    void UpdateInstance(std::uint64_t id, float dt);
    void UpdateAll(float dt);

    // Optional helper: map instance -> entity (returns 0 if not found)
    Boom::EntityId EntityOf(std::uint64_t id);

} // namespace ScriptRuntime
