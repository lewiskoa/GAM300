#include "BoomEngine.h"
#include "Vendors/imgui/imgui.h"
using namespace Boom;

struct Editor : AppInterface {};

int32_t main()
 {
    MyEngineClass engine;
    /*
    if (engine.GetWindowWeak().expired()) {
        std::cout << "window missing" << std::endl;
    }
    else {
        std::cout << "window found" << std::endl;
    }*/
    engine.whatup();
    
    BOOM_INFO("Editor Started");
	std::cout << "Editor Started" << std::endl;
    /*
    auto app{ new Application() };
    app->AttachLayer<Editor>();
    app->RunContext();
    */
    return 0;
}