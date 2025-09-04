#pragma once
#include "Callback.h"
#include "Utilities.h"

namespace Boom {
    struct PhysicsContext {
        BOOM_INLINE PhysicsContext() {
            // sinitialize physX SDK
            m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_AllocatorCallback, m_ErrorCallback);
            if (!m_Foundation)
            {
                BOOM_ERROR("Error initializing PhysX m_Foundation");
                return;
            }
            // create context instance
            m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION,
                *m_Foundation, PxTolerancesScale());
            if (!m_Physics)
            {
                BOOM_ERROR("Error initializing PhysX m_Physics");
                m_Foundation->release();
                return;
            }

            // create worker threads
            m_Dispatcher = PxDefaultCpuDispatcherCreate(2);

            // create a scene desciption
            PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
            sceneDesc.simulationEventCallback = &m_EventCallback;
            sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
            sceneDesc.filterShader = CustomFilterShader;
            sceneDesc.cpuDispatcher = m_Dispatcher;

            // create scene instance
            m_Scene = m_Physics->createScene(sceneDesc);


            if (!m_Scene)
            {
                BOOM_ERROR("Error creating PhysX m_Scene");
                m_Physics->release();
                m_Foundation->release();
                return;
            }
        }

        BOOM_INLINE ~PhysicsContext() {
            if (m_Scene) { m_Scene->release(); }
            if (m_Physics) { m_Physics->release(); }
            if (m_Dispatcher) { m_Dispatcher->release(); }
            if (m_Foundation) { m_Foundation->release(); }
        }

        // Rigid Body
        BOOM_INLINE void AddRigidBody(Entity& entity) {
            auto& transform = entity.Get<TransformComponent>().Transform;
            auto& body = entity.Get<RigidBodyComponent>().RigidBody;
            //bool hasCollider = entity.Has<ColliderComponent>();



            // create rigidbody transformation
            PxTransform pose(ToPxVec3(transform.translate));
            glm::quat rot(transform.rotate);
            pose.q = PxQuat(rot.x, rot.y, rot.z, rot.w);

            // create a rigid body actor
            if (entity.template Has<ColliderComponent>())
            {
                // create collider shape

                auto& collider = entity.Get<ColliderComponent>().Collider;

                // create collider material
                collider.Material = m_Physics -> createMaterial(collider.StaticFriction,
                collider.DynamicFriction,
                collider.Restitution);

                if (collider.Type == Collider3D::BOX)
                {
                    PxBoxGeometry
                        box(ToPxVec3(transform.scale / 2.0f));
                    collider.Shape = m_Physics -> createShape(box, *collider.Material);
                }
                else if (collider.Type == Collider3D::SPHERE)
                {
                    PxSphereGeometry
                        sphere(transform.scale.x / 2.0f);
                    collider.Shape = m_Physics -> createShape(sphere, *collider.Material);
                }
                else if (collider.Type == Collider3D::MESH)


                {
                    // coming next!
                }
                else
                {
                    BOOM_ERROR("Error creating collider invalid type provided");
                        return;
                }

                // create actor instanace

                if (body.Type == RigidBody3D::DYNAMIC)
                {
                    body.Actor = PxCreateDynamic(*m_Physics,
                        pose, *collider.Shape, body.Density);
                    body.Actor -> setActorFlag(PxActorFlag::eSEND_SLEEP_NOTIFIES, true);
                }
                else if (body.Type == RigidBody3D::STATIC)
                {
                    body.Actor = PxCreateStatic(*m_Physics,
                        pose, *collider.Shape);
                }
            }
            else
            {
                if (body.Type == RigidBody3D::DYNAMIC)
                {
                    body.Actor = m_Physics -> createRigidDynamic(pose);
                }
                else if (body.Type == RigidBody3D::STATIC)
                {
                    body.Actor = m_Physics -> createRigidStatic(pose);


                }
            }

            // check actor
            if (!body.Actor)
            {
                BOOM_ERROR("Error creating dynamic actor");
                return;
            }

            // set user data to entt id
            body.Actor->userData = new EntityID(entity.ID());

            // add actor to the m_Scene
            m_Scene->addActor(*body.Actor);
        }
    
        
             
        // Mesh Colliders
        BOOM_INLINE PxConvexMeshGeometry CookMesh(const MeshData<ShadedVert>& data) {
            // px vertex container
            std::vector<PxVec3> vertices;



            // convert position attributes
            for (auto& vertex : data.vtx)
            {
                vertices.push_back(ToPxVec3(vertex.pos));
            }

            PxConvexMeshDesc meshDesc;
            // vertices
            meshDesc.points.data = vertices.data();
            meshDesc.points.stride = sizeof(PxVec3);
            meshDesc.points.count = static_cast<PxU32>(vertices.size());
            // indices
            meshDesc.indices.data = data.idx.data();
            meshDesc.indices.count = static_cast<PxU32>(data.idx.size());
            // flags
            meshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

            // cooking the mesh
            PxCookingParams cookingParams =
                PxCookingParams(PxTolerancesScale());
            PxCooking* cooking = PxCreateCooking(PX_PHYSICS_VERSION,
                *m_Foundation, cookingParams);
            PxConvexMeshCookingResult::Enum result;
            PxConvexMesh* convexMesh = cooking -> createConvexMesh(meshDesc,
    m_Physics->getPhysicsInsertionCallback(), &result);
            PxConvexMeshGeometry convexMeshGeometry(convexMesh);

            cooking->release();
            return convexMeshGeometry;
        
        }


        BOOM_INLINE void Simulate(uint32_t step, float dt)
        {
            for (uint32_t i = 0; i < step; ++i)
            {
                // simulate m_Physics for a time step
                m_Scene->simulate(dt);

                // block until simulation is complete


                m_Scene->fetchResults(true);
            }
        }

        BOOM_INLINE void SetEventCallback(PxCallbackFunction&& callback)
        {
            m_EventCallback.m_Callback = callback;
        }



    private:
        // custom collision filter shader callback
        static PxFilterFlags CustomFilterShader
        (
            [[maybe_unused]] PxFilterObjectAttributes attributes0,
            [[maybe_unused]] PxFilterData filterData0,
            [[maybe_unused]] PxFilterObjectAttributes attributes1,
            [[maybe_unused]] PxFilterData filterData1,
            [[maybe_unused]] PxPairFlags& pairFlags, 
            [[maybe_unused]] const void* constantBlock, 
            [[maybe_unused]] PxU32 constantBlockSize
        )
        {
            (void)constantBlock;
            (void)constantBlockSize;
            // generate contacts and triggers for actors
            pairFlags |= PxPairFlag::eCONTACT_DEFAULT |
                PxPairFlag::eTRIGGER_DEFAULT;

            return PxFilterFlag::eDEFAULT;
        }



    private:
        PxDefaultErrorCallback m_ErrorCallback;
        PxDefaultAllocator m_AllocatorCallback;
        PxDefaultCpuDispatcher* m_Dispatcher;
        PxEventCallback m_EventCallback;
        PxFoundation* m_Foundation;
        PxPhysics* m_Physics;
        PxScene* m_Scene;
    };
}