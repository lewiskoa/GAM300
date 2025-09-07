#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include "Interface.h"
#include "ECS/ECS.hpp"

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
            auto view = registry.view<Components...>();
            for (auto e : view) {
                fn(EntityType{ &registry, e }, registry.get<Components>(e)...);
            }
        }
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
            CreateEntities();

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
            m_Context->window->camPos.z = 6.f;

            Camera3D cam;
            {
                auto view = m_Context->scene.view<Entity, CameraComponent>();
                for (auto ent : view) {
                    auto camera{ view.get<CameraComponent>(ent) };
                    cam = camera.camera;
                }
            }
            //init skybox
            {
                auto view{ m_Context->scene.view<Entity, SkyboxComponent>() };
                for (auto ent : view) {
                    auto& sc{ view.get<SkyboxComponent>(ent) };
                    auto& skybox{ m_Context->assets->Get<SkyboxAsset>(sc.skyboxID) };
                    m_Context->renderer->InitSkybox(skybox.data, skybox.envMap, skybox.size);
                }
            }
            //init scripts here...
            {

            }



            while (m_Context->window->PollEvents())
            {
                //updates new frame
                //RenderSceneDepth();
                m_Context->renderer->NewFrame();
                {
                    //testing rendering
                    {
                        static float testRot{};
                        if ((testRot += 0.1f) > 360.f) { testRot -= 360.f; }

                        //lights
                        m_Context->renderer->SetLight(pl1, Transform3D({ 0.f, 0.f, 3.f }, {0.f, 0.f, -1.f}, {}), 0);
                        m_Context->renderer->SetLight(pl2, Transform3D({ 1.2f, 1.2f, .5f }, {}, {}), 1);
                        m_Context->renderer->SetPointLightCount(0);

                        m_Context->renderer->SetLight(dl, Transform3D({}, { -.7f, -.3f, .3f }, {}), 0);
                        m_Context->renderer->SetDirectionalLightCount(1);

                        m_Context->renderer->SetLight(sl, Transform3D({ 0.f, 0.f, 3.f }, { 0.f, 0.f, -1.f }, {}), 0);
                        m_Context->renderer->SetSpotLightCount(0);
                        
                        /*
                        m_Context->renderer->SetCamera(cam, { m_Context->window->camPos, {0.f, testRot, 0.f}, {} });
                        
                        {
                            auto view{ m_Context->scene.view<Entity, SkyboxComponent>() };
                            for (auto ent : view) {
                                auto 
                            }
                        }

                        //testing ecs,uncomment for ecs
                        {
                            auto view = m_Context->scene.view<TransformComponent, ModelComponent>();
                            for (auto ent : view) {
                                auto& xf = view.get<TransformComponent>(ent).transform;
                                auto& mc = view.get<ModelComponent>(ent);

                                if (auto an = m_Context->scene.try_get<AnimatorComponent>(ent)) {
                                    // an is a pointer; Animator likely holds a shared_ptr<Animator>
                                    auto& joints = an->animator->Animate(0.01f); // or your real dt
                                    m_Context->renderer->SetJoints(joints);
                                    m_Context->renderer->Draw(mc.model, xf);
                                }
                                else if (mc.material) {
                                    m_Context->renderer->Draw(mc.model, xf, *mc.material);
                                }
                                else {
                                    m_Context->renderer->Draw(mc.model, xf);
                                }
                            }
                        }
                        */

                        m_Context->renderer->SetCamera(cam, { m_Context->window->camPos, {0.f, testRot, 0.f}, {} });
                        //pbr ecs
                        {
                            
                            auto view = m_Context->scene.view<Entity, ModelComponent>();
                            for (auto ent : view) {
                                auto& transform{ view.get<TransformComponent>(ent).transform };

                                auto& modelComp{ view.get<ModelComponent>(ent) };
                                auto& model{ m_Context->assets->Get<ModelAsset>(modelComp.modelID) };
                                auto& material{ m_Context->assets->Get<MaterialAsset>(modelComp.materialID) };

                                //set animator uniform if model has one
                                if (auto an = m_Context->scene.try_get<AnimatorComponent>(ent)) {
                                    auto& joints = an->animator->Animate(0.01f); // or your real dt
                                    m_Context->renderer->SetJoints(joints);
                                }

                                //draw model with material if it has one
                                if (modelComp.materialID != EMPTY_ASSET) {
                                    m_Context->renderer->Draw(model.data, transform, material.data);
                                }
                                else {
                                    m_Context->renderer->Draw(model.data, transform);
                                }
                            }
                        }
                        //skybox ecs (should be drawn at the end)
                        {
                            auto view = m_Context->scene.view<Entity, SkyboxComponent>();
                            for (auto ent : view) {
                                auto& transform{ view.get<TransformComponent>(ent).transform };

                                auto& skyComp{ view.get<SkyboxComponent>(ent) };
                                auto& skybox{ m_Context->assets->Get<SkyboxAsset>(skyComp.skyboxID) };
                                m_Context->renderer->DrawSkybox(skybox.data, transform);
                            }
                        }
     
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
            auto robotAsset{ m_Context->assets->AddModel(RandomU64(), "walking.fbx", true) };
            //script asset ...
            auto sphereAsset{ m_Context->assets->AddModel(RandomU64(), "sphere.fbx") };
            auto cubeAsset{ m_Context->assets->AddModel(RandomU64(), "cube.fbx") };
            auto mat1Asset{ m_Context->assets->AddMaterial(RandomU64(), "Marble") };
            
            //camera
            Entity camera{ &m_Context->scene };
            camera.Attach<InfoComponent>();
            camera.Attach<TransformComponent>().transform.translate.z = 20.f;
            camera.Attach<CameraComponent>();

            //skybox
            Entity skybox{ &m_Context->scene };
            skybox.Attach<InfoComponent>();
            skybox.Attach<SkyboxComponent>().skyboxID = skyboxAsset->uid;
            skybox.Attach<TransformComponent>();

            Entity robot{ &m_Context->scene };
            robot.Attach<InfoComponent>();
            auto& robotModel{ robot.Attach<ModelComponent>() };
            robotModel.materialID = mat1Asset->uid;
            robotModel.modelID = robotAsset->uid;
            auto& rt { robot.Attach<TransformComponent>().transform };
            rt.translate = glm::vec3(0.f, -2.5f, 0.f);
            rt.scale = glm::vec3(0.1f);
        }
    private:
            EntityRegistry registry;
            BOOM_INLINE void RenderSceneDepth()
            {
                EnttView<Entity, DirectLightComponent>([this](auto light, DirectLightComponent&) {
                    auto& lightDir = light.Get<TransformComponent>().Transform.rotate;

                    // begin rendering
                    m_Context->renderer->BeginShadowPass(lightDir);

                    // render depth
                    EnttView<Entity, ModelComponent>([this](auto entity, ModelComponent& comp) {
                        auto& transform = entity.Get<TransformComponent>().Transform;
                        m_Context->renderer->DrawDepth(comp.model, transform);
                        });

                    // finalize frame
                    m_Context->renderer->EndShadowPass();
                    });
            }
    };

}

#endif // !APPLICATION_H
