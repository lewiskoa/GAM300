#include "BoomEngine.h"
#include "Vendors/imgui/imgui.h"
#include "Windows/Inspector.h"
#include "Windows/Hierarchy.h"
#include "Windows/Resource.h"
#include "Windows/Viewport.h"
#include "Windows/MenuBar.h"
using namespace Boom;

//struct Editor : AppInterface {};

struct Editor : GuiContext
{
    BOOM_INLINE void OnGuiStart()
    {
        AttachWindow<ViewportWindow>();
    }
};

int32_t main()
 {
    MyEngineClass engine;
    engine.whatup();

    BOOM_INFO("Editor Started");
	std::cout << "Editor Started" << std::endl;
    /*
    auto app{ new Application() };
    app->AttachLayer<Editor>();
    app->RunContext();
    */
    //auto app = new Application();
    //app->AttachLayer<Editor>();
    //app->RunContext(false);
    auto app{ std::make_unique<Application>() };
    app->PostEvent<WindowTitleRenameEvent>("Boom Editor - Press 'Esc' to quit. 'WASD' to pan camera");
    app->RunContext();
    //BOOM_DELETE(app);
    return 0;
}