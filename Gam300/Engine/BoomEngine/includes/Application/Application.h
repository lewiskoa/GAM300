#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include "Interface.h"
#include "ECS/ECS.hpp"
#include "Physics/Context.h"
namespace Boom
{
    /**
     * @class Application
     * @brief Core application that owns the context and drives all layers.
     *
     * Inherits from AppInterface to receive the same lifecycle hooks
     * and gain access to the shared AppContext.
     */
    struct Application : AppInterface
    {
        template<typename EntityType, typename... Components, typename Fn>
        BOOM_INLINE void EnttView(Fn&& fn) {
            auto view = m_Context->scene.view<Components...>();
            for (auto e : view) {
                fn(EntityType{ &m_Context->scene, e }, m_Context->scene.get<Components>(e)...);
            }
        }
        /**
         * @brief Constructs the Application, assigns its unique ID, and allocates the AppContext.
         *
         * BOOM_INLINE hints to the compiler to inline this small constructor
         * to avoid function-call overhead during startup.
         */

        double m_SphereTimer = 0.0;
        glm::vec3 m_SphereStartPos = glm::vec3(0.0f, 5.0f, 0.0f);
        Entity m_SphereEntity; // <-- Add this


        BOOM_INLINE Application()
        {
            m_LayerID = TypeID<Application>();
            m_Context = new AppContext();
            RegisterEventCallbacks();

            AttachCallback<WindowResizeEvent>([this](auto e) {
                m_Context->renderer->Resize(e.width, e.height);
                }
            );

            AttachCallback<WindowTitleRenameEvent>([this](auto e) {
                m_Context->window->SetWindowTitle(e.title);
                }
            );
/*
            // Create a dynamic sphere entity
            Entity sphere = CreateEntt<Entity>();
            {
                // Set initial position above the cube
                auto& t = sphere.Attach<TransformComponent>().Transform;
                t.translate = m_SphereStartPos;
                t.scale = glm::vec3(1.0f); 
                // Attach rigidbody (dynamic)
                auto& rb = sphere.Attach<RigidBodyComponent>().RigidBody;
                rb.Type = RigidBody3D::DYNAMIC;
                rb.Mass = 2.0f;
                rb.Density = 1.0f;

                // Small velocity to check for movement
                rb.InitialVelocity = glm::vec3(1.0f, 0.0f, 0.0f);

                // Attach collider (sphere)
                auto& col = sphere.Attach<ColliderComponent>().Collider;
                col.Type = Collider3D::SPHERE;

                // Attach model
                auto& mc = sphere.Attach<ModelComponent>();
                mc.model = std::make_shared<StaticModel>("sphere.fbx");
            }


            // Create a static cube entity (ground)
            Entity cube = CreateEntt<Entity>();
            {
                // Set initial position at the origin
                auto& t = cube.Attach<TransformComponent>().Transform;
                t.translate = glm::vec3(0.0f, 0.0f, 0.0f);
                t.scale = glm::vec3(2.0f);

                // Attach rigidbody (static)
                auto& rb = cube.Attach<RigidBodyComponent>().RigidBody;
                rb.Type = RigidBody3D::STATIC;

                // Attach collider (box)
                auto& col = cube.Attach<ColliderComponent>().Collider;
                col.Type = Collider3D::BOX;

                // Attach model 
                auto& mc = cube.Attach<ModelComponent>();
                mc.model = std::make_shared<StaticModel>("cube.fbx");
            }

            m_SphereEntity = sphere;
            // Register both with the physics system
            m_Context->Physics->AddRigidBody(sphere);
            m_Context->Physics->AddRigidBody(cube);
*/
        }

        /**
         * @brief Destructor that frees the AppContext.
         *
         * BOOM_INLINE here helps keep the size of the vtable cleanup small
         * by inlining the delete call.
         */
        BOOM_INLINE ~Application()
        {
            DestroyPhysicsActors();
            BOOM_DELETE(m_Context);
        }

        /**
         * @brief Runs the main loop, calling OnUpdate() on every attached layer.
         *
         * BOOM_INLINE suggests inlining this hot-path entry point so
         * the call to RunContext itself adds minimal overhead.
         */
        BOOM_INLINE void RunContext()
        {
            //use of ecs
            CreateEntities();

            //lights testers
            PointLight pl1{};
            PointLight pl2{};
            DirectionalLight dl{};
            SpotLight sl{};
            {
                pl1.radiance.b = 0.f;
                pl1.intensity = 2.f;
                pl2.radiance.g = 0.f;
                pl2.intensity = 3.f;

                sl.radiance = { 1.f, 1.f, 1.f };

                dl.intensity = 10.f;
            }

            //init skybox
            EnttView<Entity, SkyboxComponent>([this](auto, auto& comp) {
                    SkyboxAsset& skybox{ m_Context->assets->Get<SkyboxAsset>(comp.skyboxID) };
                    m_Context->renderer->InitSkybox(skybox.data, skybox.envMap, skybox.size);
                }
            );
            //init scripts here...
            /*
                EnttView<Entity, ScriptComponent>([this] (auto entity, auto& comp) {
                    // retrieve scrit asset
                    auto& script = m_Context->Assets->Get<ScriptAsset>(comp.Script);
                    // load script source
                    auto name = m_Context->Scripts->LoadScript(script.Source);
                    // attach to entity
                    m_Context->Scripts->AttachScript(entity, name);
                }
            );
            */
            

            while (m_Context->window->PollEvents())
            {
                /*
                ComputeFrameDeltaTime();

                m_SphereTimer += m_Context->DeltaTime;

                // Reset sphere after 5 seconds
                if (m_SphereTimer >= 5.0)
                {
                    m_SphereTimer = 0.0;

                    // Reset position and velocity
                    auto& transform = m_SphereEntity.Get<TransformComponent>().Transform;
                    transform.translate = m_SphereStartPos;

                    auto& rb = m_SphereEntity.Get<RigidBodyComponent>().RigidBody;
                    if (rb.Actor)
                    {
                        // Reset PhysX actor pose and velocity
                        PxTransform pose(ToPxVec3(m_SphereStartPos));
                        rb.Actor->setGlobalPose(pose);

                        PxRigidDynamic* dyn = static_cast<PxRigidDynamic*>(rb.Actor);
                        if (dyn)
                        {
                            dyn->setLinearVelocity(PxVec3(1.0f, 0.0f, 0.0f)); // initial velocity
                            dyn->setAngularVelocity(PxVec3(0.0f)); // stop rotation
                            dyn->clearForce();
                            dyn->clearTorque();
                            dyn->wakeUp();
                        }
                    }
                }

                RunPhysicsSimulation();
                */
                //updates new frame
                //RenderSceneDepth();
                m_Context->renderer->NewFrame();
                {
                    //testing rendering
                    {
                        static float testRot{};
                        if ((testRot += 0.25f) > 360.f) { testRot -= 360.f; }

                        //lights
                        m_Context->renderer->SetLight(pl1, Transform3D({ 0.f, 0.f, 3.f }, { 0.f, 0.f, -1.f }, {}), 0);
                        m_Context->renderer->SetLight(pl2, Transform3D({ 1.2f, 1.2f, .5f }, {}, {}), 1);
                        m_Context->renderer->SetPointLightCount(0);

                        //glm::vec3 testDir{ -glm::cos(glm::radians(testRot)), .3f, glm::sin(glm::radians(testRot)) };
						glm::vec3 testDir{ .7f, -.3f, -.3f };
                        m_Context->renderer->SetLight(dl, Transform3D({}, testDir, {}), 0);
                        m_Context->renderer->SetDirectionalLightCount(1);

                        m_Context->renderer->SetLight(sl, Transform3D({ 0.f, 0.f, 3.f }, { 0.f, 0.f, -1.f }, {}), 0);
                        m_Context->renderer->SetSpotLightCount(0);

                        //camera
                        EnttView<Entity, CameraComponent>([this](auto entity, CameraComponent& comp) {
                                Transform3D& transform{ entity.template Get<TransformComponent>().transform };
                                transform.translate = m_Context->window->camPos;
                                transform.rotate = { 0.f, testRot, 0.f };
                                m_Context->renderer->SetCamera(comp.camera, transform);
                            }
                        );
                        
                        //pbr ecs
                        EnttView<Entity, ModelComponent>([this](auto entity, ModelComponent& comp) {
                                //set animator uniform if model has one
                                if (entity.template Has<AnimatorComponent>()) {
                                    AnimatorComponent& an{ entity.template Get<AnimatorComponent>() };
                                    auto& joints{ an.animator->Animate(0.001f) }; // or your real dt
                                    m_Context->renderer->SetJoints(joints);
                                }

                                Transform3D& transform{ entity.template Get<TransformComponent>().transform };
                                ModelAsset& model{ m_Context->assets->Get<ModelAsset>(comp.modelID) };
                                //draw model with material if it has one
                                if (comp.materialID != EMPTY_ASSET) {
                                    auto& material{ m_Context->assets->Get<MaterialAsset>(comp.materialID) };
									material.data.albedoMap = m_Context->assets->Get<TextureAsset>(material.albedoMapID).data;
                                    material.data.normalMap = m_Context->assets->Get<TextureAsset>(material.normalMapID).data;
                                    material.data.roughnessMap = m_Context->assets->Get<TextureAsset>(material.roughnessMapID).data;
                                    m_Context->renderer->Draw(model.data, transform, material.data);
                                }
                                else {
                                    m_Context->renderer->Draw(model.data, transform);
                                }
                            }
                        );
                        //skybox ecs (should be drawn at the end)
                        EnttView<Entity, SkyboxComponent>([this](auto entity, SkyboxComponent& comp) {
                                Transform3D& transform{ entity.template Get<TransformComponent>().transform };
                                SkyboxAsset& skybox{ m_Context->assets->Get<SkyboxAsset>(comp.skyboxID) };
                                m_Context->renderer->DrawSkybox(skybox.data, transform);
                            }
                        );

                    }
                }
                m_Context->renderer->EndFrame();

                //update layers
                for (auto layer : m_Context->Layers)
                {
                    layer->OnUpdate();
                }

                //draw the updated frame
                m_Context->renderer->ShowFrame();
                //glfwSwapBuffers(m_Context->window->Window());
            }
        }

        //loads assets and initialize the starting entities
        BOOM_INLINE void CreateEntities() {
            auto skyboxAsset{ m_Context->assets->AddSkybox(RandomU64(), "Skybox/sky.hdr", 2048) };
            auto robotAsset{ m_Context->assets->AddModel(RandomU64(), "dance.fbx", true) };
            //script asset ...
            auto sphereAsset{ m_Context->assets->AddModel(RandomU64(), "sphere.fbx") };
            auto cubeAsset{ m_Context->assets->AddModel(RandomU64(), "cube.fbx") };
            
            //materials
            auto albedoTexAsset{ m_Context->assets->AddTexture(RandomU64(), "Marble/albedo.png") };
            auto normalTexAsset{ m_Context->assets->AddTexture(RandomU64(), "Marble/normal.png") };
            auto roughnessTexAsset{ m_Context->assets->AddTexture(RandomU64(), "Marble/roughness.png") };
            std::array<AssetID, 6> marbleMat{
                albedoTexAsset->uid,
                normalTexAsset->uid,
                roughnessTexAsset->uid,
                EMPTY_ASSET,
                EMPTY_ASSET,
                EMPTY_ASSET,
            };
            auto mat1Asset{ m_Context->assets->AddMaterial(RandomU64(), "Marble", marbleMat) };

            //camera
            Entity camera{ &m_Context->scene };
            camera.Attach<InfoComponent>();
            camera.Attach<TransformComponent>();
            camera.Attach<CameraComponent>();

            //skybox
            Entity skybox{ &m_Context->scene };
            skybox.Attach<InfoComponent>();
            skybox.Attach<SkyboxComponent>().skyboxID = skyboxAsset->uid;
            skybox.Attach<TransformComponent>();

            //dance boi
            Entity robot{ &m_Context->scene };
            robot.Attach<InfoComponent>();
            auto& robotModel{ robot.Attach<ModelComponent>() };
            robotModel.materialID = mat1Asset->uid;
            robotModel.modelID = robotAsset->uid;
            auto& rt { robot.Attach<TransformComponent>().transform };
            rt.translate = glm::vec3(0.f, -1.5f, 0.f);
            rt.scale = glm::vec3(0.01f);
            robot.Attach<AnimatorComponent>().animator = std::dynamic_pointer_cast<SkeletalModel>(robotAsset->data)->GetAnimator();
        
            //sphere 
			Entity sphereEn{ &m_Context->scene };
			sphereEn.Attach<InfoComponent>();
			auto& sphereModel{ sphereEn.Attach<ModelComponent>() };
			sphereModel.modelID = sphereAsset->uid;
			sphereEn.Attach<TransformComponent>().transform.translate = glm::vec3(0.f, 1.f, 0.f);
        }
private:
        BOOM_INLINE void RegisterEventCallbacks()
        {
            // Set physics event callback (mark unused param to avoid warnings)
            m_Context->Physics->SetEventCallback([this](auto e)
                {
                    (void)e; 
                    // Scripting/event logic can be added here
                });

            // Attach window resize event callback
            AttachCallback<WindowResizeEvent>([this](auto e)
                {
                    m_Context->renderer->Resize(e.width, e.height);
                });
        }
BOOM_INLINE void ComputeFrameDeltaTime()
        {
            static double sLastTime = glfwGetTime();
            double currentTime = glfwGetTime();
            m_Context->DeltaTime = (currentTime - sLastTime);
            sLastTime = currentTime;
        }

        BOOM_INLINE void DestroyPhysicsActors()
        {
            EnttView<Entity, RigidBodyComponent>([this](auto entity, auto& comp)
                {
                    if (entity.template Has<ColliderComponent>())
                    {
                        auto& collider = entity.template Get<ColliderComponent>().Collider;
                        collider.Material->release();
                        collider.Shape->release();
                    }
                    // Destroy actor user data
                    EntityID* owner = static_cast<EntityID*>(comp.RigidBody.Actor->userData);
                    BOOM_DELETE(owner);
                    // Destroy actor instance
                    comp.RigidBody.Actor->release();
                });
        }

        BOOM_INLINE void RunPhysicsSimulation()
        {
            m_Context->Physics->Simulate(1, static_cast<float>(m_Context->DeltaTime));
            EnttView<Entity, RigidBodyComponent>([this](auto entity, auto& comp)
                {
                    auto& transform = entity.template Get<TransformComponent>().transform;
                    auto pose = comp.RigidBody.Actor->getGlobalPose();
                    glm::quat rot(pose.q.x, pose.q.y, pose.q.z, pose.q.w);
                    transform.rotate = glm::degrees(glm::eulerAngles(rot));
                    transform.translate = PxToVec3(pose.p);
                });
        }
    };

}

#endif // !APPLICATION_H
