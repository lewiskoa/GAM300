#include "Core.h"
#include "Application/Context.h"
#include "Application/Interface.h"

namespace Boom
{
    AppContext::~AppContext()
    {
        for (AppInterface*& layer : Layers)
        {
            BOOM_DELETE(layer);
        }
    }
}