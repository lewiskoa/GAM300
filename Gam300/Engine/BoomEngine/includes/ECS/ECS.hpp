#pragma once
#include <entt/entt.hpp>
#include "Graphics/Utilities/Data.h"
#include "Auxiliaries/Assets.h"
#include "Physics/Utilities.h"  
namespace Boom {
	using EntityRegistry = entt::registry;  
	using EntityID = entt::entity;
	constexpr EntityID NENTT = entt::null;

  

    // transform component
    struct TransformComponent
    {
        BOOM_INLINE TransformComponent(const TransformComponent&) = default;
        BOOM_INLINE TransformComponent() = default;
        Transform3D transform;
    };

   
    // camera component
    struct CameraComponent
    {
        BOOM_INLINE CameraComponent(const CameraComponent&) = default;
        BOOM_INLINE CameraComponent() = default;
        Camera3D camera;
    };

    struct EnttComponent {
        BOOM_INLINE EnttComponent(const EnttComponent&) = default;
        BOOM_INLINE EnttComponent() = default;
        std::string name = "Entity";
    };

	struct MeshComponent
	{
		BOOM_INLINE MeshComponent(const MeshComponent&) = default;
		BOOM_INLINE MeshComponent() = default;
		Mesh3D mesh;
	};
    
    struct RigidBodyComponent
    {
        BOOM_INLINE RigidBodyComponent(const RigidBodyComponent&) = default;
        BOOM_INLINE RigidBodyComponent() = default;
        RigidBody3D RigidBody;
	};

    struct ColliderComponent
    {
        BOOM_INLINE ColliderComponent(const ColliderComponent&) = default;
        BOOM_INLINE ColliderComponent() = default;
        Collider3D Collider;
	};

    ////Model Component
    struct ModelComponent {
        AssetID materialID{ EMPTY_ASSET };
        AssetID modelID{ EMPTY_ASSET };
    };

	//Animator Component
    struct AnimatorComponent
    {
		BOOM_INLINE AnimatorComponent(const AnimatorComponent&) = default;
		BOOM_INLINE AnimatorComponent() = default;
		Animator3D animator;
    };

    struct SkyboxComponent {
        AssetID skyboxID{ EMPTY_ASSET };
    };

    //helpful for encapsulating information about an entity
    //can be used for entity hierarchies (linked-list)
    struct InfoComponent {
        AssetID parent{ EMPTY_ASSET };
        std::string name{ "Entity" };
        AssetID uid{ RandomU64() };
    };
    struct DirectLightComponent
    {
        BOOM_INLINE DirectLightComponent(const DirectLightComponent&) = default;
        BOOM_INLINE DirectLightComponent() = default;
        DirectionalLight light;
    };
    struct PointLightComponent
    {
        BOOM_INLINE PointLightComponent(const PointLightComponent&) = default;
        BOOM_INLINE PointLightComponent() = default;
        PointLight light;
    };
    struct SpotLightComponent
    {
        BOOM_INLINE SpotLightComponent(const SpotLightComponent&) = default;
        BOOM_INLINE SpotLightComponent() = default;
        SpotLight light;
	};
    struct Entity
    {
        BOOM_INLINE Entity(EntityRegistry* registry, EntityID entity) :
            m_Registry(registry), m_EnttID(entity)
        {
        }

        BOOM_INLINE Entity(EntityRegistry* registry) :
            m_Registry(registry)
        {
            m_EnttID = m_Registry->create();
        }

        BOOM_INLINE virtual ~Entity() = default;
        BOOM_INLINE Entity() = default;

        BOOM_INLINE operator EntityID ()
        {
            return m_EnttID;
        }

        BOOM_INLINE operator bool()
        {
            return m_Registry != nullptr &&
                m_Registry->valid(m_EnttID);
        }

        BOOM_INLINE EntityID ID()
        {
            return m_EnttID;
        }


        template<typename T, typename... Args>
        BOOM_INLINE T& Attach(Args&&... args)
        {
            return m_Registry->get_or_emplace<T>
                (m_EnttID, std::forward<Args>(args)...);
        }

        template<typename T>
        BOOM_INLINE void Detach()
        {
            m_Registry->remove<T>(m_EnttID);
        }

        BOOM_INLINE void Destroy()
        {
            if (m_Registry)
            {
                m_Registry->destroy(m_EnttID);
            }
        }

        template<typename T>
        BOOM_INLINE bool Has() const
        {
            return m_Registry != nullptr &&
                m_Registry->all_of<T>(m_EnttID);
        }

        template<typename T>
        BOOM_INLINE T& Get()
        {
            return m_Registry->get<T>(m_EnttID);
        }
    
    protected:
        EntityRegistry* m_Registry = nullptr;
        EntityID m_EnttID = NENTT;
    };
   
}
