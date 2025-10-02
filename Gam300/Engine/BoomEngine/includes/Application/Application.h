#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include "Interface.h"
#include "ECS/ECS.hpp"
#include "Physics/Context.h"
#include "Audio/Audio.hpp"   
#include "Auxiliaries/DataSerializer.h"

namespace Boom
{
    /**
     * @enum ApplicationState
     * @brief Defines the current state of the application
     */
    enum class ApplicationState
    {
        RUNNING,
        PAUSED,
        STOPPED
    };

    /**
     * @class Application
     * @brief Core application that owns the context and drives all layers.
     *
     * Inherits from AppInterface to receive the same lifecycle hooks
     * and gain access to the shared AppContext.
     */
    struct 
        
        
        
    Application : AppInterface
    {
        template<typename EntityType, typename... Components, typename Fn>
        BOOM_INLINE void EnttView(Fn&& fn) {
            auto view = m_Context->scene.view<Components...>();
            for (auto e : view) {
                fn(EntityType{ &m_Context->scene, e }, m_Context->scene.get<Components>(e)...);
            }
        }
        
        double m_SphereTimer = 0.0;

        // Application state management
        ApplicationState m_AppState = ApplicationState::RUNNING;
        double m_PausedTime = 0.0;  // Track time spent paused
        double m_LastPauseTime = 0.0;  // When the last pause started
        bool m_ShouldExit = false;  // Flag for graceful shutdown
        float m_TestRot = 0.0f;


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
            RegisterEventCallbacks();
			

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
            DestroyPhysicsActors();
            BOOM_DELETE(m_Context);
            //called here in case of the need of multiple windows
            glfwTerminate();
        }

        /**
        * @brief Pauses the application (stops updates but continues rendering)
        */
        BOOM_INLINE void Pause()
        {
            if (m_AppState == ApplicationState::RUNNING) {
                m_AppState = ApplicationState::PAUSED;
                m_LastPauseTime = glfwGetTime();
                BOOM_INFO("[Application] Paused");

                // Pause audio if available
            }
        }

        /**
         * @brief Resumes the application from pause
         */
        BOOM_INLINE void Resume()
        {
            if (m_AppState == ApplicationState::PAUSED) {
                m_AppState = ApplicationState::RUNNING;

                // Add paused time to total paused time
                m_PausedTime += (glfwGetTime() - m_LastPauseTime);

                BOOM_INFO("[Application] Resumed");

                // Resume audio if available
            }
        }

        /**
         * @brief Stops the application gracefully
         */
        BOOM_INLINE void Stop()
        {
            m_AppState = ApplicationState::STOPPED;
            m_ShouldExit = true;
            BOOM_INFO("[Application] Stopping application...");

            // Stop audio if available
        }

        /**
         * @brief Toggles between pause and resume
         */
        BOOM_INLINE void TogglePause()
        {
            if (m_AppState == ApplicationState::RUNNING) {
                Pause();
            }
            else if (m_AppState == ApplicationState::PAUSED) {
                Resume();
            }
        }

        /**
         * @brief Gets the current application state
         */
        BOOM_INLINE ApplicationState GetState() const { return m_AppState; }

        /**
         * @brief Gets the adjusted time (excluding paused time)
         */
        BOOM_INLINE double GetAdjustedTime() const
        {
            double currentTime = glfwGetTime();
            double adjustedPausedTime = m_PausedTime;

            // If currently paused, add the current pause duration
            if (m_AppState == ApplicationState::PAUSED) {
                adjustedPausedTime += (currentTime - m_LastPauseTime);
            }

            return currentTime - adjustedPausedTime;
        }


        /**
         * @brief Runs the main loop, calling OnUpdate() on every attached layer.
         *
         * BOOM_INLINE suggests inlining this hot-path entry point so
         * the call to RunContext itself adds minimal overhead.
         */
        BOOM_INLINE void RunContext(bool showFrame = false)
        {
            LoadScene("default");

            ////init skybox
            EnttView<Entity, SkyboxComponent>([this](auto, auto& comp) {
                SkyboxAsset& skybox{ m_Context->assets->Get<SkyboxAsset>(comp.skyboxID) };
                m_Context->renderer->InitSkybox(skybox.data, skybox.envMap, skybox.size);
                });

            //temp input for mouse motion
            glm::dvec2 curMP{};
            glm::dvec2 prevMP{};
            while (m_Context->window->PollEvents() && !m_ShouldExit)
            {
                BOOM_INFO("[MainLoop] Frame starting"); // ADD THIS
                std::shared_ptr<GLFWwindow> engineWindow = m_Context->window->Handle();

                glfwMakeContextCurrent(engineWindow.get());

                // Always update delta time, but adjust for pause state
                ComputeFrameDeltaTime();

                m_Context->profiler.BeginFrame();
                m_Context->profiler.Start("Total Frame");
                m_Context->profiler.Start("Renderer Start Frame");
                m_Context->renderer->NewFrame();
                m_Context->profiler.End("Renderer Start Frame");

                // Only update rotation when running
                if (m_AppState == ApplicationState::RUNNING) {
                    
                }
                // When paused, m_TestRot stays at its current value

                //lights (always set up)
               /* m_Context->renderer->SetLight(pl1, Transform3D({ 0.f, 0.f, 3.f }, { 0.f, 0.f, -1.f }, {}), 0);
                m_Context->renderer->SetLight(pl2, Transform3D({ 1.2f, 1.2f, .5f }, {}, {}), 1);
                m_Context->renderer->SetPointLightCount(0);

                glm::vec3 testDir{ -.7f, -.3f, .3f };
                m_Context->renderer->SetLight(dl, Transform3D({}, testDir, {}), 0);
                m_Context->renderer->SetDirectionalLightCount(1);

                m_Context->renderer->SetLight(sl, Transform3D({ 0.f, 0.f, 3.f }, { 0.f, 0.f, -1.f }, {}), 0);
                m_Context->renderer->SetSpotLightCount(0);*/
               
                {
                    int points = 0;
                    EnttView<Entity, PointLightComponent, TransformComponent>(
                        [this, &points](auto, PointLightComponent& plc, TransformComponent& tc)
                        {
                            m_Context->renderer->SetLight(plc.light, tc.transform, points++);
                        });
                    m_Context->renderer->SetPointLightCount(points);
                }
				{
					int directs = 0;
					EnttView<Entity, DirectLightComponent, TransformComponent>(
						[this, &directs](auto, DirectLightComponent& dlc, TransformComponent& tc)
						{
							m_Context->renderer->SetLight(dlc.light, tc.transform, directs++);
						});
					m_Context->renderer->SetDirectionalLightCount(directs);
				}
				{
					int spots = 0;
					EnttView<Entity, SpotLightComponent, TransformComponent>(
						[this, &spots](auto, SpotLightComponent& slc, TransformComponent& tc)
						{
							m_Context->renderer->SetLight(slc.light, tc.transform, spots++);
						});
					m_Context->renderer->SetSpotLightCount(spots);
				}               
             

                //temp input for mouse motion
                glfwGetCursorPos(m_Context->window->Handle().get(), &curMP.x, &curMP.y);

                //camera (always set up, but rotation freezes when paused)
                EnttView<Entity, CameraComponent>([this, &curMP, &prevMP](auto entity, CameraComponent& comp) {
                    Transform3D& transform{ entity.template Get<TransformComponent>().transform };

                    //get dir vector of current camera
                    transform.rotate.x += m_Context->window->camRot.x;
                    transform.rotate.y += m_Context->window->camRot.y;
                    glm::quat quat{ glm::radians(transform.rotate) };
                    glm::vec3 dir{ quat * m_Context->window->camMoveDir };
                    transform.translate += dir;

                    comp.camera.FOV = m_Context->window->camFOV;
                    if (curMP == prevMP) {
                        m_Context->window->camRot = {};
                        if (m_Context->window->isMiddleClickDown)
                            m_Context->window->camMoveDir = {};
                    }

                    m_Context->renderer->SetCamera(comp.camera, transform);
                });
                prevMP = curMP;

                //pbr ecs (always render)
                EnttView<Entity, ModelComponent>([this](auto entity, ModelComponent& comp) {
                    static int renderCount = 0;
                    static bool debugModelsPrinted = false;

                    if (!debugModelsPrinted && renderCount < 5) {
                        BOOM_INFO("[Render] Rendering model entity, ModelID: {}, MaterialID: {}",
                            comp.modelID, comp.materialID);
                    }

                    ModelAsset& model{ m_Context->assets->Get<ModelAsset>(comp.modelID) };

                    if (!model.data) {
                        BOOM_ERROR("[Render] Model data is null for ModelID: {} ({})",
                            comp.modelID, model.name);
                        return; // Skip rendering this model
                    }

                    //set animator uniform if model has one
                    if (entity.template Has<AnimatorComponent>()) {
                        AnimatorComponent& an{ entity.template Get<AnimatorComponent>() };
                        // Only animate if not paused
                        float deltaTime = (m_AppState == ApplicationState::RUNNING) ? static_cast<float>(m_Context->DeltaTime) : 0.0f;
                        auto& joints{ an.animator->Animate(deltaTime) };
                        m_Context->renderer->SetJoints(joints);
                    }

                    Transform3D& transform{ entity.template Get<TransformComponent>().transform };
                    //ModelAsset& model{ m_Context->assets->Get<ModelAsset>(comp.modelID) };

                    if (!debugModelsPrinted && renderCount < 5) {
                        BOOM_INFO("[Render] Transform position: ({}, {}, {}), scale: ({}, {}, {})",
                            transform.translate.x, transform.translate.y, transform.translate.z,
                            transform.scale.x, transform.scale.y, transform.scale.z);
                    }

                    //draw model with material if it has one
                    if (comp.materialID != EMPTY_ASSET) {
                        auto& material{ m_Context->assets->Get<MaterialAsset>(comp.materialID) };

                        // Only assign textures if they exist and are valid
                        if (material.albedoMapID != EMPTY_ASSET) {
                            auto& albedoTex = m_Context->assets->Get<TextureAsset>(material.albedoMapID);
                            if (albedoTex.data) {
                                material.data.albedoMap = albedoTex.data;
                            }
                        }
                        if (material.normalMapID != EMPTY_ASSET) {
                            auto& normalTex = m_Context->assets->Get<TextureAsset>(material.normalMapID);
                            if (normalTex.data) {
                                material.data.normalMap = normalTex.data;
                            }
                        }
                        if (material.roughnessMapID != EMPTY_ASSET) {
                            auto& roughnessTex = m_Context->assets->Get<TextureAsset>(material.roughnessMapID);
                            if (roughnessTex.data) {
                                material.data.roughnessMap = roughnessTex.data;
                            }
                        }

                        m_Context->renderer->Draw(model.data, transform, material.data);
                    }

                    renderCount++;
                    if (renderCount >= 5) debugModelsPrinted = true;
                    });

                //skybox ecs (should be drawn at the end)
                EnttView<Entity, SkyboxComponent>([this](auto entity, SkyboxComponent& comp) {
                    Transform3D& transform{ entity.template Get<TransformComponent>().transform };
                    SkyboxAsset& skybox{ m_Context->assets->Get<SkyboxAsset>(comp.skyboxID) };
                    m_Context->renderer->DrawSkybox(skybox.data, transform);
                    });

                m_Context->profiler.Start("Renderer End Frame");
                m_Context->renderer->EndFrame();
                m_Context->profiler.End("Renderer End Frame");

                //draw the updated frame
                m_Context->renderer->ShowFrame(showFrame);


                for (auto layer : m_Context->Layers)
                {
                    layer->OnUpdate();
                }
                //update layers only when running
                if (m_AppState == ApplicationState::RUNNING)
                {
                    // Run physics simulation only when running
                    RunPhysicsSimulation();
                }

                m_Context->profiler.End("Total Frame");
                m_Context->profiler.EndFrame();
            }
        }



        /**
     * @brief Saves the current scene and assets to files
     * @param sceneName The name of the scene (without extension)
     * @param scenePath Optional custom path for scene files (defaults to "Scenes/")
     * @return true if save was successful, false otherwise
     */
        BOOM_INLINE bool SaveScene(const std::string& sceneName, const std::string& scenePath = "Scenes/")
        {
            //Try blocks cause crashed in release mode. Need to find new alternative
            DataSerializer serializer;

            const std::string sceneFilePath = scenePath + sceneName + ".yaml";
            const std::string assetsFilePath = scenePath + sceneName + "_assets.yaml";

            BOOM_INFO("[Scene] Saving scene '{}' to '{}'", sceneName, sceneFilePath);

            // Serialize scene and assets
            serializer.Serialize(m_Context->scene, sceneFilePath);
            serializer.Serialize(*m_Context->assets, assetsFilePath);

            // Update current scene tracking
            strncpy_s(m_CurrentScenePath, sizeof(m_CurrentScenePath), sceneFilePath.c_str(), _TRUNCATE);

            BOOM_INFO("[Scene] Successfully saved scene '{}' and assets", sceneName);
            return true;
        }

        /**
         * @brief Loads a scene from files, replacing the current scene
         * @param sceneName The name of the scene (without extension)
         * @param scenePath Optional custom path for scene files (defaults to "Scenes/")
         * @return true if load was successful, false otherwise
         */
        BOOM_INLINE bool LoadScene(const std::string& sceneName, const std::string& scenePath = "Scenes/")
        {
            DataSerializer serializer;

            const std::string sceneFilePath = scenePath + sceneName + ".yaml";
            const std::string assetsFilePath = scenePath + sceneName + "_assets.yaml";

            BOOM_INFO("[Scene] Loading scene '{}' from '{}'", sceneName, sceneFilePath);

            // Clean up current scene
            CleanupCurrentScene();

            // Load assets first
            BOOM_INFO("[Scene] Loading assets...");
            serializer.Deserialize(*m_Context->assets, assetsFilePath);

            // Then load scene
            BOOM_INFO("[Scene] Loading scene data...");
            serializer.Deserialize(m_Context->scene, *m_Context->assets, sceneFilePath);

            // Update tracking
            strncpy_s(m_CurrentScenePath, sizeof(m_CurrentScenePath), sceneFilePath.c_str(), _TRUNCATE);
            m_SceneLoaded = true;

            // Reinitialize systems that need it
            ReinitializeSceneSystems();

            BOOM_INFO("[Scene] Successfully loaded scene '{}'", sceneName);
            return true;
        }


        /**
         * @brief Creates a new empty scene
         * @param sceneName Optional name for the new scene
         */
        BOOM_INLINE void NewScene(const std::string& sceneName = "NewScene")
        {
            BOOM_INFO("[Scene] Creating new scene '{}'", sceneName);

            // Clean up current scene
            CleanupCurrentScene();

            // Create basic scene with camera
            CreateDefaultScene();

            m_CurrentScenePath[0] = '\0'; // Clear the path
            m_SceneLoaded = false;

            BOOM_INFO("[Scene] New scene '{}' created", sceneName);
        }

        /**
         * @brief Gets the current scene file path
         */
        BOOM_INLINE std::string GetCurrentScenePath() const { return std::string(m_CurrentScenePath); }

        /**
         * @brief Checks if a scene is currently loaded
         */
        BOOM_INLINE bool IsSceneLoaded() const { return m_SceneLoaded; }

    private:

        char m_CurrentScenePath[512] = "\0";
        bool m_SceneLoaded = false;

        /**
     * @brief Cleans up the current scene and physics actors
     */
        BOOM_INLINE void CleanupCurrentScene()
        {
            BOOM_INFO("[Scene] Cleaning up current scene...");

            // Destroy physics actors before clearing scene
            DestroyPhysicsActors();

            // Clear the ECS scene
            m_Context->scene.clear();

            // Reset asset registry (keeping EMPTY_ASSET sentinels)
            *m_Context->assets = AssetRegistry();

            // Reset any scene-specific state
            

            BOOM_INFO("[Scene] Scene cleanup complete");
        }

        /**
         * @brief Reinitializes systems after loading a scene
         */
        BOOM_INLINE void ReinitializeSceneSystems()
        {
            BOOM_INFO("[Scene] Reinitializing scene systems...");

            EnttView<Entity, SkyboxComponent>([this](auto, auto& comp) {
                SkyboxAsset& skybox{ m_Context->assets->Get<SkyboxAsset>(comp.skyboxID) };
                m_Context->renderer->InitSkybox(skybox.data, skybox.envMap, skybox.size);
                BOOM_INFO("[Scene] Reinitialized skybox");
                });

            // Only reinitialize physics
            EnttView<Entity, RigidBodyComponent>([this](auto entity, auto&) {
                m_Context->Physics->AddRigidBody(entity, *m_Context->assets);
                });

            BOOM_INFO("[Scene] Scene systems reinitialization complete");
        }

        /**
         * @brief Creates a minimal default scene with camera
         */
        BOOM_INLINE void CreateDefaultScene()
        {
            BOOM_INFO("[Scene] Creating default scene...");

            // Create basic camera entity
            Entity camera{ &m_Context->scene };
            camera.Attach<InfoComponent>();
            camera.Attach<TransformComponent>();
            camera.Attach<CameraComponent>();

            BOOM_INFO("[Scene] Default scene created with camera");
        }

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

            // Calculate raw delta time
            double rawDelta = (currentTime - sLastTime);

            // Only update delta time if not paused
            if (m_AppState == ApplicationState::RUNNING) {
                m_Context->DeltaTime = rawDelta;
            }
            else {
                m_Context->DeltaTime = 0.0; // No time progression when paused
            }

            sLastTime = currentTime;
        }

        BOOM_INLINE void DestroyPhysicsActors()
        {
            EnttView<Entity, RigidBodyComponent>([this](auto entity, auto& comp)
                {
                    if (entity.template Has<ColliderComponent>())
                    {
                        auto& collider = entity.template Get<ColliderComponent>().Collider;
                        collider.material->release();
                        collider.Shape->release();
                    }
                    // Destroy actor user data
                    EntityID* owner = static_cast<EntityID*>(comp.RigidBody.actor->userData);
                    BOOM_DELETE(owner);
                    // Destroy actor instance
                    comp.RigidBody.actor->release();
                });
        }

        BOOM_INLINE void RunPhysicsSimulation()
        {
            // Only simulate physics if running
            if (m_AppState == ApplicationState::RUNNING)
            {
                m_Context->Physics->Simulate(1, static_cast<float>(m_Context->DeltaTime));
                EnttView<Entity, RigidBodyComponent>([this](auto entity, auto& comp)
                    {
                        auto& transform = entity.template Get<TransformComponent>().transform;
                        auto pose = comp.RigidBody.actor->getGlobalPose();
                        glm::quat rot(pose.q.x, pose.q.y, pose.q.z, pose.q.w);
                        transform.rotate = glm::degrees(glm::eulerAngles(rot));
                        transform.translate = PxToVec3(pose.p);
                    });
            }
        }
    };

}

#endif // !APPLICATION_H
