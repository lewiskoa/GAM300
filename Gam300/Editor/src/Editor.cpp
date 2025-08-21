#include "BoomEngine.h"
#include "AppWindow.h"
#include "Graphics/Renderer.h"

int32_t main()
 {
    MyEngineClass engine;
    engine.whatup();

    BOOM_INFO("Editor Started");

    //idk appInterface usage
    Boom::AppWindow awin{1800, 900, "Boom Editor"};
    Boom::GraphicsRenderer g{ 1800, 900 };
    while (true) {
        if (awin.IsExit()) break;

        awin.OnUpdate();
        g.OnUpdate();
    }
    return 0;
}