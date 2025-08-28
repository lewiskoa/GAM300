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
            auto model = std::make_shared<Model>("Resources/Models/Cube.fbx");
            while (m_Context->window->PollEvents())
            {
                //updates new frame
                m_Context->renderer->NewFrame();
                {
                    //testing rendering
                    {
                        m_Context->renderer->Draw(model, Transform3D({ {}, {}, glm::vec3{0.5f} }));
                    }
                    /*
                    //set shader cam
                    EnttView<Entity, CameraComponent>([this](auto entity, auto& comp) {
                            auto& transform{entity.templateGet<TransformComponent>().Transform};
                            m_Context->renderer->SetCamera(comp.Camera, transform);
                        }
                    );

                    //render models
                    EnttView<Entity, MeshComponent>([this](auto entity, auto& comp) {
                            auto& transform{entity.templateGet<TransformComponent>().Transform};
                            m_Context->renderer->Draw(comp.Mesh, transform);
                        }
                    );
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
            }
        }
    };

}

#endif // !APPLICATION_H
