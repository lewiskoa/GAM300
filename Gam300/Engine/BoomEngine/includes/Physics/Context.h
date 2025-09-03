#pragma once
#include "Callback.h"
#include "Utilities.h"

namespace Boom {
	struct PhysicsContext {
		BOOM_INLINE PhysicsContext() {
            // create worker threads
            m_Dispatcher = PxDefaultCpuDispatcherCreate(2);

            // create a scene desciption
            PxSceneDesc sceneDesc(m_Physics -> getTolerancesScale());
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



    private:
        // custom collision filter shader callback
        static PxFilterFlags CustomFilterShader
        (
            PxFilterObjectAttributes attributes0,
            PxFilterData filterData0,
            PxFilterObjectAttributes attributes1,
            PxFilterData filterData1,
            PxPairFlags& pairFlags, const void*
            constantBlock, PxU32 constantBlockSize
        )
        {
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