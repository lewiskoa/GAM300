// includes/Scripting/ScriptAPI.h
#pragma once
#include "Core.h"           // defines BOOM_API
#include <stdint.h>         // uint64_t

#ifdef __cplusplus
extern "C" {
#endif

    typedef uint32_t ScriptEntityId;

    typedef struct ScriptVec3 {
        float x, y, z;
    } ScriptVec3;

    /* Delegates (C function pointers) */
    typedef void (*ScriptCreateFn)  (ScriptEntityId /*entity*/, uint64_t /*instanceId*/);
    typedef void (*ScriptUpdateFn)  (uint64_t /*instanceId*/, float /*dt*/);
    typedef void (*ScriptDestroyFn) (uint64_t /*instanceId*/);

    /* -------- Logging -------- */
    BOOM_API void           script_log(const char* msg);

    /* -------- Minimal ECS surface -------- */
    BOOM_API ScriptEntityId script_create_entity(void);
    BOOM_API void           script_destroy_entity(ScriptEntityId e);

    /* -------- Transform -------- */
    BOOM_API void           script_set_position(ScriptEntityId e, ScriptVec3 p);
    BOOM_API ScriptVec3     script_get_position(ScriptEntityId e);

    /* -------- Registration & lifetime -------- */
    BOOM_API void           script_register_type(const char* typeName,
        ScriptCreateFn c,
        ScriptUpdateFn u,
        ScriptDestroyFn d);

    BOOM_API uint64_t       script_create_instance(const char* typeName, ScriptEntityId e);
    BOOM_API void           script_destroy_instance(uint64_t instanceId);
    BOOM_API void           script_update_instance(uint64_t instanceId, float dt);
    BOOM_API void           script_update_all(float dt);

#ifdef __cplusplus
} /* extern "C" */
#endif
