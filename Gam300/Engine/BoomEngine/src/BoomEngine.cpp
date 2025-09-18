// BoomEngine.cpp : Defines the functions for the static library.
//
//Build BoomEngine before running debuger or building Gam300
#include "BoomEngine.h"
#include "Core.h"
#include "framework.h"
#include "common/Events.h"

#include "AppWindow.h"
#include "Graphics/Renderer.h"
#include "Graphics/Shaders/Color.h"
#include "Graphics/Buffers/Frame.h"
#include "ECS/ECS.hpp"
#include <iostream>
#include "Audio/Audio.hpp"
#include <imgui.h>
using namespace std;
using namespace Boom;
#define LOG_EVENT(MSG) std::cout << "[Event] " << MSG << '\n'
// TODO: This is an example of a library function

namespace {
    //this function shows the barebones process of entire rendering process
    //void TestRender();
    void UpdateAudioTest();
    //this function shows the renderer logic
   // void TestShaders(Boom::EventDispatcher& dispatcher);
}

void MyEngineClass::whatup() {
    cout << "nig\n";

	// Example of using the logger
    #ifdef BOOM_ENABLE_LOG
        BOOM_INFO("Logger is active!");
        BOOM_DEBUG("DEBUG TEST");
        BOOM_WARN("WARNING TEST");
    #else
        std::cout << "BOOM_ENABLE_LOG not defined." << std::endl;
    #endif
        static EventDispatcher dispatcher;          // one per library instance

     
        struct WindowResizeEvent
        {
            int w, h;
           
            WindowResizeEvent(int width, int height) : w(width), h(height) {}
        };
        struct QuitEvent {};

        uint32_t resizeID = 42;
        dispatcher.AttachCallback<WindowResizeEvent>(
            [](const WindowResizeEvent& e)
            { LOG_EVENT("Resize -> " << e.w << 'x' << e.h); },
            resizeID);

        dispatcher.AttachCallback<QuitEvent>(
            [](const QuitEvent&) { LOG_EVENT("Quit requested"); },
            /*listenerID=*/7);

      
        dispatcher.PostEvent<WindowResizeEvent>(1280, 720);
        dispatcher.PostEvent<QuitEvent>();
        dispatcher.PostTask([] { LOG_EVENT("Frame task ran"); });

    
        dispatcher.PollEvents();

        
        dispatcher.DetachCallback<WindowResizeEvent>(resizeID);
        dispatcher.PostEvent<WindowResizeEvent>(1920, 1080);
        dispatcher.PollEvents();

        std::cout << "Dispatcher smoketest finished inside MyEngineClass::whatup().\n";

        {
            //TestShaders(dispatcher);
            //actual application code to run
            auto app{ std::make_unique<Application>() };
            app->PostEvent<WindowTitleRenameEvent>("Boom Editor - Press 'Esc' to quit. 'WASD' to pan camera");

            auto& se = SoundEngine::Instance();
            if (se.Init()) {
                se.PlaySound("startup", "Resources/Audio/vboom.wav", false);

                se.PlaySound("orbit", "Resources/Audio/Fetty Wap.wav", true);

                auto start = std::chrono::high_resolution_clock::now();
                while (true) {
                    FMOD_VECTOR listenerPos = { 0.0f, 0.0f, 0.0f }; // Listener at origin
                    FMOD_VECTOR listenerVel = { 0.0f, 0.0f, 0.0f };
                    FMOD_VECTOR listenerForward = { 0.0f, 0.0f, 1.0f }; // Facing +Z
                    FMOD_VECTOR listenerUp = { 0.0f, 1.0f, 0.0f }; // Y-up

                    SoundEngine::Instance().SetListenerAttributes(listenerPos, listenerVel, listenerForward, listenerUp);

                    se.Update();         // FMOD update
                    UpdateAudioTest();   // orbiting test
                    //std::this_thread::sleep_for(std::chrono::milliseconds(16));
                }
                //// Simple loop to update FMOD for 2 seconds
                //auto start = std::chrono::high_resolution_clock::now();
                //while (std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count() < 2.0f) {
                //    se.Update();
                //    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                //}
            }
            app->RunContext();

            SoundEngine::Instance().Shutdown();
        }
}

namespace {
    std::string GetGlewString(GLenum name, bool isError = false) {
        if (isError)
            return reinterpret_cast<char const*>(glewGetErrorString(name));
        else {
            char const* ret{ reinterpret_cast<char const*>(glewGetString(name)) };
            return ret ? ret : "Unknown glewGetString(" + std::to_string(name) + ')';
        }
    }

    void UpdateAudioTest() {
        static float angle = 0.0f;
        angle += 0.02f; // orbit speed
        float radius = 8.0f;

        FMOD_VECTOR pos = {
            cos(angle) * radius, // X
            0.0f,                // Y
            sin(angle) * radius  // Z
        };

        FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };

        SoundEngine::Instance().SetSoundPosition("orbit", pos, vel);
    }

    void PrintSpecs() {
        BOOM_INFO("GPU Vendor: {}", GetGlewString(GL_VENDOR));
        BOOM_INFO("GPU Renderer: {}", GetGlewString(GL_RENDERER));
        BOOM_INFO("GPU Version: {}", GetGlewString(GL_VERSION));
        BOOM_INFO("GPU Shader Version: {}", GetGlewString(GL_SHADING_LANGUAGE_VERSION));

        GLint ver[2];
        glGetIntegerv(GL_MAJOR_VERSION, &ver[0]);
        glGetIntegerv(GL_MINOR_VERSION, &ver[1]);
        BOOM_INFO("GL Version: {}.{}", ver[0], ver[1]);

        GLboolean isDB;
        glGetBooleanv(GL_DOUBLEBUFFER, &isDB);
        if (isDB)
            BOOM_INFO("Current OpenGL Context is double-buffered");
        else
            BOOM_INFO("Current OpenGL Context is not double-buffered");

        GLint output;
        glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &output);
        BOOM_INFO("Maximum Vertex Count: {}", output);
        glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &output);
        BOOM_INFO("Maximum Indicies Count: {}", output);
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &output);
        BOOM_INFO("Maximum texture size: {}", output);

        GLint viewport[2];
        glGetIntegerv(GL_MAX_VIEWPORT_DIMS, viewport);
        BOOM_INFO("Maximum Viewport Dimensions: {} x {}", viewport[0], viewport[1]);

        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &output);
        BOOM_INFO("Maximum generic vertex attributes: {}", output);
        glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &output);
        BOOM_INFO("Maximum vertex buffer bindings: {}\n", output);
    }
    //void TestRender() {
    //    if (!glfwInit()) {
    //        BOOM_FATAL("AppWindow::Init() - glfwInit() failed.");
    //        std::exit(EXIT_FAILURE);
    //    }
    //    GLFWwindow* window = glfwCreateWindow(1800, 900, "test", NULL, NULL);
    //    if (!window)
    //    {
    //        std::cerr << "GLFW failed to create window context." << std::endl;
    //        glfwTerminate();
    //        std::exit(EXIT_FAILURE);
    //    }

    //    glfwMakeContextCurrent(window);

    //    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    //    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    //    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    //    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    //    glfwWindowHint(GLFW_RED_BITS, 8); glfwWindowHint(GLFW_GREEN_BITS, 8);
    //    glfwWindowHint(GLFW_BLUE_BITS, 8); glfwWindowHint(GLFW_ALPHA_BITS, 8);

    //    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //no resize of app window

    //    auto keyCB = [](GLFWwindow* win, int32_t key, int32_t, int32_t action, int32_t) {
    //        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    //            glfwSetWindowShouldClose(win, GLFW_TRUE);
    //            return;
    //        }
    //        };
    //    glfwSetKeyCallback(window, keyCB);
    //    glfwSetErrorCallback([](int32_t errorNo, char const* description) {
    //        BOOM_ERROR("[GLFW]: [{}] {}", errorNo, description);
    //        });

    //    glfwSwapInterval(-1);

    //    glewExperimental = GL_TRUE;
    //    GLenum err = glewInit();
    //    if (GLEW_OK != err) {
    //        std::cerr << "Unable to initialize GLEW - error: " << glewGetErrorString(err) << " abort program" << std::endl;
    //        std::exit(EXIT_FAILURE);
    //    }
    //    if (GLEW_VERSION_4_5) {
    //        std::cout << "Using glew version: " << glewGetString(GLEW_VERSION) << std::endl;
    //        std::cout << "Driver supports OpenGL 4.5\n" << std::endl;
    //    }
    //    else {
    //        std::cerr << "Warning: The driver may lack full compatibility with OpenGL 4.5, potentially limiting access to advanced features." << std::endl;
    //    }

    //    PrintSpecs();

    //    //store mesh buffer data
    //    ColorShader s{
    //        "color.glsl",
    //        {1.f, 0.f, 0.f, 1.f}
    //    };

    //    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //    glEnable(GL_BLEND);
    //    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //    glViewport(0, 0, 1800, 900);
    //    std::apply(glClearColor, CONSTANTS::DEFAULT_BACKGROUND_COLOR);

    //    while (!glfwWindowShouldClose(window)) {
    //        //glClear(GL_COLOR_BUFFER_BIT);

    //        {//draw quad model (triangle strip)
    //            s.Show();
    //        }

    //        glfwSwapBuffers(window);
    //        glfwPollEvents();
    //    }
    //    glfwDestroyWindow(window);
    //    glfwTerminate();
    //}
    //void TestShaders(Boom::EventDispatcher& dispatcher) {
    //    Boom::AppWindow awin{ &dispatcher, CONSTANTS::WINDOW_WIDTH, CONSTANTS::WINDOW_HEIGHT, "Boom Editor - Press 'Esc' to quit" };
    //    Boom::GraphicsRenderer g{ CONSTANTS::WINDOW_WIDTH, CONSTANTS::WINDOW_HEIGHT };
    //    //color shader uses a test quad that only covers a portion of the screen
    //    ColorShader cs{
    //            "color.glsl",
    //            {1.f, 1.f, 0.f, .5f} //translucent yellow
    //    };
    //    FinalShader fs{
    //        "final.glsl",
    //        {1.f, 0.f, 0.f, 1.f} //red
    //    };
    //    //PBRShader pbrs{ "pbr.glsl" };
    //    FrameBuffer fb{ 1800, 900 };
    //    Color3DShader cs3d{
    //        "color3D.glsl",
    //        {1.f, 1.f, 0.f, 0.5f} //translucent yellow
    //    };

    //    while (awin.PollEvents()) {
    //        //update all system logic
    //        {
    //            //...
    //            dispatcher.PollEvents();
    //        }

    //        //finally render logic
    //        {
    //            glClear(GL_COLOR_BUFFER_BIT);
    //            //load frame buffer for texture for final 2d shader
    //            fb.Begin();
    //            //pbrs.Use();
    //            {
    //                //this portion is where ecs is to be placed
    //            }
    //            //pbrs.UnUse();
    //            fb.End();

    //            //display the 2nd passed shader + test color shader onto the window

    //            fs.Show(fb.GetTexture());
    //            //cs.Show();
    //            cs3d.Show();

    //            glfwSwapBuffers(awin.Window());
    //        }
    //    }
    //}
}