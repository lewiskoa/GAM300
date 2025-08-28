#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include "Interface.h"

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
            /*
            //create scene cam
            auto cam{ CreateEntt<Entity>() };
            camera.Attach<TransformComponent>().Transform.Translate.z = 2.f;
            camera.Attach<CameraComponent>();

            { //chose one example entity to render by simply commenting out the other one.
                //create cube entity (.fbx file reading)
                auto model = std::make_shared<Model>("Resources/Models/{filename}.fbx");
                auto cube = CreateEntt<Entity>();
                cube.Attach<TransformComponent>().Transform.Rotation.y = 30.f;
                cube.Attach<ModelComponent>().Model = model;

                //create quad
                auto quad{ CreateEntt<Entity>() };
                quad.Attach<MeshComponent>().Mesh = CreateQuad3D();
                quad.Attach<TransformComponent>();
            }
            */
            Camera3D cam{};
            auto model = std::make_shared<Model>(std::string(CONSTANTS::MODELS_LOCAITON) + "cube.fbx");
            float testRotCam{}; //TEST VARIABLE: TO BE REMOVED
            while (m_Context->window->PollEvents())
            {
                //updates new frame
                m_Context->renderer->NewFrame();
                {
                    //testing rendering
                    {
                        if ((testRotCam += 0.1f) > 360.f) { testRotCam -= 360.f; }
                        m_Context->renderer->SetCamera(cam, {m_Context->window->camPos, {0.f, 0.f, testRotCam}, {}});
                        //cube model, (translate,rotate,scale), default material(red, rough, slight metal)
                        m_Context->renderer->Draw(model, Transform3D({ {0.f, 0.f, -2.f}, {testRotCam, 30.f, testRotCam}, glm::vec3{1.f} }));
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

}

#endif // !APPLICATION_H
