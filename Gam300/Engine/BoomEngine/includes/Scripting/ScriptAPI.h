#pragma once
#include "GlobalConstants.h" // Boom::EntityId, Boom::Vec3

extern "C" {

	// plain C struct for ABI
	struct ScriptVec3 { float x, y, z; };

	// C# delegates (must match managed signatures)
	using CreateFn = void(*)(Boom::EntityId /*entity*/, std::uint64_t /*instanceId*/);
	using UpdateFn = void(*)(std::uint64_t /*instanceId*/, float /*dt*/);
	using DestroyFn = void(*)(std::uint64_t /*instanceId*/);

	// -------- Logging
	BOOM_API void           script_log(const char* msg);

	// -------- Minimal ECS surface
	BOOM_API Boom::EntityId script_create_entity();
	BOOM_API void           script_destroy_entity(Boom::EntityId e);

	// -------- Transform
	BOOM_API void           script_set_position(Boom::EntityId e, ScriptVec3 p);
	BOOM_API ScriptVec3     script_get_position(Boom::EntityId e);

	// -------- Registration & lifetime
	BOOM_API void           script_register_type(const char* typeName, CreateFn c, UpdateFn u, DestroyFn d);
	BOOM_API std::uint64_t  script_create_instance(const char* typeName, Boom::EntityId e);
	BOOM_API void           script_destroy_instance(std::uint64_t instanceId);
	BOOM_API void           script_update_instance(std::uint64_t instanceId, float dt);
	BOOM_API void           script_update_all(float dt);

} // extern "C"
