// BoomEngine.cpp : Defines the functions for the static library.
//
//Build BoomEngine before running debuger or building Gam300
#include "BoomEngine.h"
#include "Core.h"
#include "framework.h"
#include "common/Events.h"
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <imgui.h>
using namespace std;
using namespace Boom;
#define LOG_EVENT(MSG) std::cout << "[Event] " << MSG << '\n'
// TODO: This is an example of a library function
void MyEngineClass::whatup() {
    cout << "nig\n";
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
}
