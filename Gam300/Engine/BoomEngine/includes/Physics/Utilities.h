#pragma once
#include "Helpers.h"

namespace Boom {
    struct RigidBody3D
    {
        BOOM_INLINE RigidBody3D(const RigidBody3D&) = default;
        BOOM_INLINE RigidBody3D() = default;

        // body actor pointer
        PxRigidActor* actor = nullptr;

        // body density
        float density = 1.0f;

        float mass = 1.0f;
        glm::vec3 initialVelocity = glm::vec3(0.0f); // Default to zero
        // rigidbody type
        enum Type{
            DYNAMIC = 0,
            STATIC,
        } type;
    };

    struct Collider3D
    {
        BOOM_INLINE Collider3D(const Collider3D&) = default;
        BOOM_INLINE Collider3D() = default;



        // collider matrial pointer
        PxMaterial* material = nullptr;

        // collider material data
        float dynamicFriction = 0.5f;
        float staticFriction = 0.0f;
        float restitution = 0.1f;

        // mesh for custom shape
        PxConvexMeshGeometry mesh;

        // collider geometry shape
        PxShape* Shape = nullptr;

        // collider shape type
        enum Type{
            BOX = 0,
            SPHERE,
            MESH
        } type;
    };
}