#pragma once
#ifdef _WIN32
#define SCRIPT_API extern "C" __declspec(dllexport)
#else
#define SCRIPT_API extern "C"
#endif

#include "Core.h"

struct Vec3 { float x, y, z; };
using Entity = uint32_t;

// Logging and basic entity operations
SCRIPT_API void script_log(const char* msg);
SCRIPT_API Entity script_create_entity();
SCRIPT_API void script_set_position(Entity e, Vec3 p);
SCRIPT_API Vec3 script_get_position(Entity e);

// Script lifecycle registration
using CreateFn = void(*)(Entity, uint64_t);
using UpdateFn = void(*)(uint64_t, float);
using DestroyFn = void(*)(uint64_t);
SCRIPT_API void script_register_type(const char* typeName, CreateFn c, UpdateFn u, DestroyFn d);

// Instance control
SCRIPT_API uint64_t script_create_instance(const char* typeName, Entity e);
SCRIPT_API void     script_destroy_instance(uint64_t id);
SCRIPT_API void     script_update_instance(uint64_t id, float dt);
