#include "BoomEngine.h"
using namespace Boom;

struct Editor : AppInterface {};

int32_t main()
 {
    MyEngineClass engine;
    engine.whatup();

    BOOM_INFO("Editor Started");
    /*
    auto app{ new Application() };
    app->AttachLayer<Editor>();
    app->RunContext();
    */
    return 0;
}