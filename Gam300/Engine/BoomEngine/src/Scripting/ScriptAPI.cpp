#include "Core.h"                    // keep first if using PCH
#include "Scripting/ScriptAPI.h"
#include "Scripting/ScriptRuntime.h"
#include <string>

static inline Boom::Vec3  to_engine(const ScriptVec3& v) { return { v.x, v.y, v.z }; }
static inline ScriptVec3  to_abi(const Boom::Vec3& v) { return { v.x, v.y, v.z }; }

extern "C" {

    BOOM_API void script_log(const char* msg) {
        if (ScriptRuntime::Hooks().Log) ScriptRuntime::Hooks().Log(msg ? msg : "");
    }

    BOOM_API Boom::EntityId script_create_entity() {
        return ScriptRuntime::Hooks().CreateEntity ? ScriptRuntime::Hooks().CreateEntity() : 0;
    }

    BOOM_API void script_destroy_entity(Boom::EntityId e) {
        if (ScriptRuntime::Hooks().DestroyEntity) ScriptRuntime::Hooks().DestroyEntity(e);
    }

    BOOM_API void script_set_position(Boom::EntityId e, ScriptVec3 p) {
        if (ScriptRuntime::Hooks().SetPosition) ScriptRuntime::Hooks().SetPosition(e, to_engine(p));
    }

    BOOM_API ScriptVec3 script_get_position(Boom::EntityId e) {
        return ScriptRuntime::Hooks().GetPosition ? to_abi(ScriptRuntime::Hooks().GetPosition(e))
            : ScriptVec3{ 0,0,0 };
    }

    BOOM_API void script_register_type(const char* typeName, CreateFn c, UpdateFn u, DestroyFn d) {
        ScriptRuntime::RegisterType(typeName ? std::string(typeName) : std::string(), c, u, d);
    }

    BOOM_API std::uint64_t script_create_instance(const char* typeName, Boom::EntityId e) {
        return ScriptRuntime::CreateInstance(typeName ? std::string(typeName) : std::string(), e);
    }

    BOOM_API void script_destroy_instance(std::uint64_t instanceId) {
        ScriptRuntime::DestroyInstance(instanceId);
    }

    BOOM_API void script_update_instance(std::uint64_t instanceId, float dt) {
        ScriptRuntime::UpdateInstance(instanceId, dt);
    }

    BOOM_API void script_update_all(float dt) {
        ScriptRuntime::UpdateAll(dt);
    }

} // extern "C"
