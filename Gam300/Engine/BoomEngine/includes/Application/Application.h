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
            m_Context->window->camPos.z = 6.f;

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
                                    auto& joints{ an.animator->Animate(0.01f) }; // or your real dt
                                    m_Context->renderer->SetJoints(joints);
                                }

                                Transform3D& transform{ entity.template Get<TransformComponent>().transform };
                                ModelAsset& model{ m_Context->assets->Get<ModelAsset>(comp.modelID) };
                                //draw model with material if it has one
                                if (comp.materialID != EMPTY_ASSET) {
                                    auto& material{ m_Context->assets->Get<MaterialAsset>(comp.materialID) };
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
            auto robotAsset{ m_Context->assets->AddModel(RandomU64(), "walking.fbx", true) };
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
                    auto& lightDir = light.Get<TransformComponent>().transform.rotate;

                    // begin rendering
                    m_Context->renderer->BeginShadowPass(lightDir);

                    // render depth
                    EnttView<Entity, ModelComponent>([this](auto entity, ModelComponent& comp) {
                        auto& transform = entity.Get<TransformComponent>().transform;
                        ModelAsset& model{ m_Context->assets->Get<ModelAsset>(comp.modelID) };
                        m_Context->renderer->DrawDepth(model.data, transform);
                        });

                    // finalize frame
                    m_Context->renderer->EndShadowPass();
                    });
            }
    };

}

#endif // !APPLICATION_H
