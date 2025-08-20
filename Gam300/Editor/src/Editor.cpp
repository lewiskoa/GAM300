#include "BoomEngine.h"
#include "AppWindow.h"

int32_t main()
 {
    MyEngineClass engine;
    engine.whatup();

    BOOM_INFO("Editor Started");

    Boom::AppWindow awin;
    awin.Init();
    //auto terminate
    return 0;
}