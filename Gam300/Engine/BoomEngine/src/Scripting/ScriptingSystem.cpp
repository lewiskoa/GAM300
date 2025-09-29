#include "Core.h"
#include "Scripting/ScriptingSystem.h"

void ScriptingSystem::Init(const ScriptRuntime::EngineHooks& hooks) {
    ScriptRuntime::Initialize(hooks);     // or ScriptRuntime::SetHooks(hooks)
}

void ScriptingSystem::Shutdown() {
    ScriptRuntime::Shutdown();
}

void ScriptingSystem::Update(float dt) {
    ScriptRuntime::UpdateAll(dt);
}

std::uint64_t ScriptingSystem::Create(const char* typeName, Boom::EntityId e) {
    return ScriptRuntime::CreateInstance(typeName, e);
}

void ScriptingSystem::Destroy(std::uint64_t id) {
    ScriptRuntime::DestroyInstance(id);
}
