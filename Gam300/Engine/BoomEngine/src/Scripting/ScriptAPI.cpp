#include "Core.h"                  // BOOM_API
#include "GlobalConstants.h"       // Boom::EntityId, Boom::Vec3
#include "Scripting/ScriptRuntime.h"
#include "Scripting/ScriptAPI.h"   // C ABI (prototypes)

static inline Boom::EntityId to_engine_id(ScriptEntityId id) {
    static_assert(sizeof(ScriptEntityId) == sizeof(Boom::EntityId),
        "ScriptEntityId must match Boom::EntityId size");
    return static_cast<Boom::EntityId>(id);
}
static inline ScriptEntityId to_script_id(Boom::EntityId id) {
    return static_cast<ScriptEntityId>(id);
}

extern "C" {

    /* =========================
       Engine-facing operations
       (through EngineHooks)
       ========================= */

    BOOM_API void script_log(const char* msg)
    {
        if (ScriptRuntime::Hooks().Log)
            ScriptRuntime::Hooks().Log(msg ? msg : "");
    }

    BOOM_API ScriptEntityId script_create_entity(void)
    {
        return ScriptRuntime::Hooks().CreateEntity
            ? to_script_id(ScriptRuntime::Hooks().CreateEntity())
            : (ScriptEntityId)0;
    }

    BOOM_API void script_destroy_entity(ScriptEntityId e)
    {
        if (ScriptRuntime::Hooks().DestroyEntity)
            ScriptRuntime::Hooks().DestroyEntity(to_engine_id(e));
    }

    BOOM_API void script_set_position(ScriptEntityId e, ScriptVec3 p)
    {
        if (ScriptRuntime::Hooks().SetPosition)
            ScriptRuntime::Hooks().SetPosition(to_engine_id(e), Boom::Vec3{ p.x, p.y, p.z });
    }

    BOOM_API ScriptVec3 script_get_position(ScriptEntityId e)
    {
        if (ScriptRuntime::Hooks().GetPosition) {
            Boom::Vec3 v = ScriptRuntime::Hooks().GetPosition(to_engine_id(e));
            return ScriptVec3{ v.x, v.y, v.z };
        }
        return ScriptVec3{ 0.0f, 0.0f, 0.0f };
    }

    /* ===============================================
       Script runtime registry & instance management
       (directly call ScriptRuntime API, NOT hooks)
       =============================================== */

    BOOM_API void script_register_type(const char* typeName,
        ScriptCreateFn c,
        ScriptUpdateFn u,
        ScriptDestroyFn d)
    {
        // forward to the runtime registry (thread-safe inside)
        ScriptRuntime::RegisterType(typeName, c, u, d);
    }

    BOOM_API uint64_t script_create_instance(const char* typeName, ScriptEntityId e)
    {
        return ScriptRuntime::CreateInstance(typeName, to_engine_id(e));
    }

    BOOM_API void script_destroy_instance(uint64_t instanceId)
    {
        ScriptRuntime::DestroyInstance(instanceId);
    }

    BOOM_API void script_update_instance(uint64_t instanceId, float dt)
    {
        // (bugfix from your version: don’t check SetPosition; just forward)
        ScriptRuntime::UpdateInstance(instanceId, dt);
    }

    BOOM_API void script_update_all(float dt)
    {
        ScriptRuntime::UpdateAll(dt);
    }

    BOOM_API void script_physics_set_gravity(ScriptVec3 g)
    {
        if (ScriptRuntime::Hooks().PhysicsSetGravity)
            ScriptRuntime::Hooks().PhysicsSetGravity(Boom::Vec3{ g.x, g.y, g.z });
    }

    BOOM_API void script_add_rigidbody(ScriptEntityId e, float mass)
    {
        if (ScriptRuntime::Hooks().AddRigidbody)
            ScriptRuntime::Hooks().AddRigidbody(to_engine_id(e), mass);
    }

    BOOM_API void script_add_box_collider(ScriptEntityId e, ScriptVec3 he)
    {
        if (ScriptRuntime::Hooks().AddBoxCollider)
            ScriptRuntime::Hooks().AddBoxCollider(to_engine_id(e), Boom::Vec3{ he.x, he.y, he.z });
    }

    BOOM_API void script_add_sphere_collider(ScriptEntityId e, float radius)
    {
        if (ScriptRuntime::Hooks().AddSphereCollider)
            ScriptRuntime::Hooks().AddSphereCollider(to_engine_id(e), radius);
    }

    BOOM_API void script_set_linear_velocity(ScriptEntityId e, ScriptVec3 v)
    {
        if (ScriptRuntime::Hooks().SetLinearVelocity)
            ScriptRuntime::Hooks().SetLinearVelocity(to_engine_id(e), Boom::Vec3{ v.x, v.y, v.z });
    }

    BOOM_API ScriptVec3 script_get_linear_velocity(ScriptEntityId e)
    {
        if (ScriptRuntime::Hooks().GetLinearVelocity) {
            Boom::Vec3 v = ScriptRuntime::Hooks().GetLinearVelocity(to_engine_id(e));
            return ScriptVec3{ v.x, v.y, v.z };
        }
        return ScriptVec3{ 0, 0, 0 };
    }

    BOOM_API void script_physics_step(float dt)
    {
        if (ScriptRuntime::Hooks().PhysicsStep)
            ScriptRuntime::Hooks().PhysicsStep(dt);
    }

} // extern "C"
