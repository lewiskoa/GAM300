#pragma once
#include <string>
// No need to include Mono headers here; keep the header light.
#include "Scripting/MonoRuntime.h"

namespace Boom {

    // Forward declare the internal-call registration function so you
    // don't need to include ScriptBindings.h here.
    struct AppContext;

#ifdef _MSC_VER
    // Suppress “needs to have dll-interface” noise for STL members in an exported class.
#pragma warning(push)
#pragma warning(disable:4251) // std::string dll-interface
#pragma warning(disable:4275) // base class dll-interface (harmless for this case)
#endif

    class BOOM_API ScriptingSystem {
    public:
        // call from Editor on startup with the folder that contains GameScripts.dll
        bool Init(const std::string& scriptsDir);
        void Shutdown();

        bool LoadScriptsDll(const std::string& dllPath);
        bool CallStart();               // calls GameScripts.Entry:Start()
        bool CallUpdate(float dt);      // calls GameScripts.Entry:Update(float)

    private:
        MonoRuntime  m_Mono;
        MonoAssembly* m_Scripts = nullptr;
        std::string  m_ScriptsDir;
    };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace Boom
