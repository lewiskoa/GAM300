#pragma once
#include "Core.h"
#include "Scripting/ScriptRuntime.h"

// A thin engine-facing wrapper that orchestrates ScriptRuntime.
// (Nothing here redefines hooks or globals.)
class ScriptingSystem {
public:
    void Init(const ScriptRuntime::EngineHooks& hooks);
    void Shutdown();
    void Update(float dt);

    // Optional sugar helpers for your engine/game code:
    std::uint64_t Create(const char* typeName, Boom::EntityId e);
    void          Destroy(std::uint64_t id);
};
