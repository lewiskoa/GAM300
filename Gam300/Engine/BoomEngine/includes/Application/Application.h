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
        /**
         * @brief Constructs the Application, assigns its unique ID, and allocates the AppContext.
         *
         * BOOM_INLINE hints to the compiler to inline this small constructor
         * to avoid function-call overhead during startup.
         */
        BOOM_INLINE Application()
        {
            m_LayerID = TypeID<Application>();
            m_Context = new AppContext();

            AttachCallback<WindowResizeEvent>([this](auto e) {
                m_Context->renderer->Resize(e.width, e.height);
                }
            );

            AttachCallback<WindowTitleRenameEvent>([this](auto e) {
                m_Context->window->SetWindowTitle(e.title);
                }
            );
        }

        /**
         * @brief Destructor that frees the AppContext.
         *
         * BOOM_INLINE here helps keep the size of the vtable cleanup small
         * by inlining the delete call.
         */
        BOOM_INLINE ~Application()
        {
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
            //to do aqif check code and implement entt here

            //create scene cam
   /*         auto cam{ createentt<entity>() };
            camera.attach<transformcomponent>().transform.translate.z = 2.f;
            camera.attach<cameracomponent>();*/

            //{ //chose one example entity to render by simply commenting out the other one.
            //    //create cube entity (.fbx file reading)
            //    auto model = std::make_shared<model>("sphere.fbx");
            //    auto cube = createentt<entity>();
            //    cube.attach<transformcomponent>().transform.rotation.y = 30.f;
            //    cube.attach<modelcomponent>().model = model;

            //    //create quad
            //    auto quad{ createentt<entity>() };
            //    quad.attach<meshcomponent>().mesh = createquad3d();
            //    quad.attach<transformcomponent>();
            //}
            //use of ecs
            EntityRegistry registry;

            Entity sphere{ &registry };
            {
                auto& t = sphere.Attach<TransformComponent>().Transform;
                t.rotate.y = 30.f;

                auto& mc = sphere.Attach<ModelComponent>();
                mc.model = std::make_shared<Model>("sphere.fbx");
            }

            Camera3D cam{};
            //this .fbx cube's normals is a little janky
            auto modelCube = std::make_shared<Model>("cube.fbx");
            auto modelSphere = std::make_shared<Model>("sphere.fbx");

            //lights testers
            PointLight pl1{};
            PointLight pl2{};
            DirectionalLight dl{};
            SpotLight sl{};
            {
                //pl1.radiance.b = 0.f;
                //pl1.intensity = 2.f;
                //pl2.radiance.g = 0.f;
                //pl2.intensity = 3.f;

                sl.radiance = { 1.f, 1.f, 1.f };

                dl.intensity = 10.f;
            }
            m_Context->window->camPos.z = 3.f;

            //textures
            auto roughness = std::make_shared<Texture2D>("Marble/roughness.png");
            auto albedo = std::make_shared<Texture2D>("Marble/albedo.png");
            auto normal = std::make_shared<Texture2D>("Marble/normal.png");

            PbrMaterial mat{};
            {
                mat.metallic = 0.15f;
                mat.roughnessMap = roughness;
                mat.albedoMap = albedo;
                mat.normalMap = normal;
            }

            auto skymap = std::make_shared<Texture2D>("HDR/sky.hdr", true);
            Skybox skybox{};
            m_Context->renderer->InitSkybox(skybox, skymap, 2048);

            while (m_Context->window->PollEvents())
            {
                //updates new frame
                m_Context->renderer->NewFrame();
                {
                    //testing rendering
                    {
                        static float testRot{};
                        if ((testRot += 0.1f) > 360.f) { testRot -= 360.f; }

                        //lights
                        m_Context->renderer->SetLight(pl1, Transform3D({ 0.f, 0.f, 3.f }, { 0.f, 0.f, -1.f }, {}), 0);
                        m_Context->renderer->SetLight(pl2, Transform3D({ 1.2f, 1.2f, .5f }, {}, {}), 1);
                        m_Context->renderer->SetPointLightCount(0);

                        m_Context->renderer->SetLight(dl, Transform3D({}, { -cosf(glm::radians(testRot)), -.3f, sinf(glm::radians(testRot)) }, {}), 0);
                        m_Context->renderer->SetDirectionalLightCount(1);

                        m_Context->renderer->SetLight(sl, Transform3D({ 0.f, 0.f, 3.f }, { 0.f, 0.f, -1.f }, {}), 0);
                        m_Context->renderer->SetSpotLightCount(0);

                        //camera
                        m_Context->renderer->SetCamera(cam, { m_Context->window->camPos, {0.f, 0.f, 0.f}, {} });

                        //models
                        /*
                        m_Context->renderer->Draw(
                            modelCube,
                            Transform3D({2.f, 0.f, -1.f}, {}, glm::vec3{1.f})
                        );*/
                        //testing ecs,uncomment for ecs
                        {
                            auto view = registry.view<TransformComponent, ModelComponent>();
                            for (auto ent : view) {
                                auto& xf = view.get<TransformComponent>(ent).Transform;
                                auto& mc = view.get<ModelComponent>(ent);

                                if (mc.material) {
                                    m_Context->renderer->Draw(mc.model, xf, *mc.material);
                                }
                                else {
                                    m_Context->renderer->Draw(mc.model, xf);
                                }
                            }
                        }

                        //comment this out/remove if using ecs
                        m_Context->renderer->Draw(
                            modelSphere,
                            Transform3D({}, {}, glm::vec3{ 2.f }),
                            mat //using custom material
                        );

                        //skybox should be drawn at the end
                        m_Context->renderer->DrawSkybox(skybox, Transform3D({}, { 0.f, testRot, 0.f }, {}));
                    }
                    /*
                    //set shader cam
                    EnttView<Entity, CameraComponent>([this](auto entity, auto& comp) {
                            auto& transform{entity.templateGet<TransformComponent>().Transform};
                            m_Context->renderer->SetCamera(comp.camera, transform);
                        }
                    );

                    { //like before, comment out/remove non used code
                        //render mesh
                        EnttView<Entity, MeshComponent>([this](auto entity, auto& comp) {
                                auto& transform{entity.templateGet<TransformComponent>().Transform};
                                m_Context->renderer->Draw(comp.mesh, transform);
                            }
                        );
                        //or render model
                        EnttView<Entity, ModelCOmponent>([this](auto entity, auto& comp) {
                                auto& transform{entity.templateGet<TransformComponent>().Transform};
                                m_Context->renderer->Draw(comp.model, transform, comp.material);
                            }
                        );
                    }
                    */
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
    };

    //    struct Application : AppInterface
    //    {
    //        // creates application context
    //        BOOM_INLINE Application()
    //        {
    //            // initialize app context
    //            m_LayerID = TypeID<Application>();
    //            m_Context = new AppContext();
    //            // register event callbacks
    //            RegisterEventCallbacks();
    //            // create scene entities
    //            //CreateSceneEntities();
    //            // create physics actors
    //            CreatePhysicsActors();
    //            // create environm. maps
    //            CreateSkyboxEnvMaps();
    //        }
    //        // destroy application context
    //        BOOM_INLINE ~Application()
    //        {
    //            // release physics actors
    //            DestroyPhysicsActors();
    //            BOOM_DELETE(m_Context);
    //        }
    //        // runs application main loop
    //        BOOM_INLINE void RunContext()
    //        {
    //            // application main loop
    //		    while (m_Context->window->PollEvents())
    //            {
    //                // compute delta time value
    //                ComputeFrameDeltaTime();
    //                // run physics simulation
    //                RunPhysicsSimulation();
    //                // render scene shadow map
    //                RenderSceneDepthMap();
    //                // render scene to buffer
    //                //RenderSceneToFBO();
    //                // update all layers
    //                UpdateAppLayers();
    //            }
    //        }
    //    private:
    //        BOOM_INLINE void RegisterEventCallbacks()
    //        {
    //            // set physics event callback
    //            m_Context->Physics->SetEventCallback([this](auto [[maybe_unused]] e)
    //                {
    //                    // coming later with scripting
    //                });
    //            // attach window resize event callback
    //            AttachCallback<WindowResizeEvent>([this](auto e)
    //                {
    //                    m_Context->renderer->Resize(e.width, e.height);
    //                });
    //        }
    //
    //        BOOM_INLINE void ComputeFrameDeltaTime()
    //        {
    //            static double sLastTime = glfwGetTime();
    //            double currentTime = glfwGetTime();
    //            m_Context->DeltaTime = (currentTime - sLastTime);
    //            sLastTime = currentTime;
    //        }
    //
    //        BOOM_INLINE void RunPhysicsSimulation()
    //        {
    //            // compute physx
    //            m_Context->Physics->Simulate(1, m_Context->DeltaTime);
    //            // start physics
    //            EnttView<Entity, RigidBodyComponent>([this](auto entity,
    //                auto& comp)
    //                {
    //                    auto& transform = entity.template
    //                        Get<TransformComponent>().Transform;
    //                    auto pose = comp.RigidBody.Actor->getGlobalPose();
    //                    glm::quat rot(pose.q.x, pose.q.y, pose.q.z,
    //                        pose.q.w);
    //                    transform.rotate =
    //                        glm::degrees(glm::eulerAngles(rot));
    //                    transform.translate = PxToVec3(pose.p);
    //                });
    //        }
    //
    //        BOOM_INLINE void DestroyPhysicsActors()
    //        {
    //            EnttView<Entity, RigidBodyComponent>([this](auto entity,
    //                auto& comp)
    //                {
    //                    if (entity.template Has<ColliderComponent>())
    //                    {
    //                        auto& collider = entity.template
    //                            Get<ColliderComponent>().Collider;
    //                        collider.Material->release();
    //                        collider.Shape->release();
    //                    }
    //                    // destroy actor user data
    //                    EntityID* owner = static_cast<EntityID*>
    //                        (comp.RigidBody.Actor->userData);
    //                    BOOM_DELETE(owner);
    //                    // destroy actor instance
    //                    comp.RigidBody.Actor->release();
    //                });
    //        }
    //        BOOM_INLINE void CreateSkyboxEnvMaps() {/* ... */ }
    //        BOOM_INLINE void RenderSceneDepthMap() {/* ... */ }
    //        BOOM_INLINE void CreatePhysicsActors() {
    //            EnttView<Entity, RigidBodyComponent>([this](auto entity,
    //                auto& comp)
    //                {
    //                    m_Context->Physics->AddRigidBody(entity);
    //                });
    //        }
    //
    //
    //
    //        BOOM_INLINE void UpdateAppLayers() {
    //            for (auto layer : m_Context->Layers)
    //            {
    //                layer->OnUpdate();
    //            }
    //            // show scene to screen
    //            m_Context->renderer->ShowFrame();
    //        }
    //
    //
    //
    //    };
    
}

#endif // !APPLICATION_H
