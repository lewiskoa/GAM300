#include "Scripting/ScriptAPI.h"     // the C ABI header C# sees
#include "Scripting/ScriptRuntime.h"  // internal glue only used by native

#include <string>

// ----------------- C ABI implementation -----------------

// Logging
void script_log(const char* msg)
{
    if (ScriptRuntime::Hooks().Log) ScriptRuntime::Hooks().Log(msg ? msg : "");
}

// Basic ECS surface
Entity script_create_entity()
{
    if (ScriptRuntime::Hooks().CreateEntity) return ScriptRuntime::Hooks().CreateEntity();
    return 0;
}

void script_set_position(Entity e, Vec3 p)
{
    if (ScriptRuntime::Hooks().SetPosition) ScriptRuntime::Hooks().SetPosition(e, p);
}

Vec3 script_get_position(Entity e)
{
    if (ScriptRuntime::Hooks().GetPosition) return ScriptRuntime::Hooks().GetPosition(e);
    return { 0,0,0 };
}

// Registration of script types (called by C# ScriptRuntime.RegisterBehaviours)
void script_register_type(const char* typeName, CreateFn c, UpdateFn u, DestroyFn d)
{
    ScriptRuntime::RegisterType(typeName ? std::string(typeName) : std::string(), c, u, d);
}

// Instance control (used by your Editor / scene loading)
uint64_t script_create_instance(const char* typeName, Entity e)
{
    return ScriptRuntime::CreateInstance(typeName ? std::string(typeName) : std::string(), e);
}

void script_destroy_instance(uint64_t instanceId)
{
    ScriptRuntime::DestroyInstance(instanceId);
}

void script_update_instance(uint64_t instanceId, float dt)
{
    ScriptRuntime::UpdateInstance(instanceId, dt);
}

void script_update_all(float dt)
{
    ScriptRuntime::UpdateAll(dt);
}
