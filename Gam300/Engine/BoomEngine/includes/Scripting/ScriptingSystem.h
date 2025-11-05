#pragma once
#include <string>
#include "Scripting/MonoRuntime.h"

namespace Boom {

    class BOOM_API ScriptingSystem {
    public:
        bool Init(const std::string& scriptsDir);     // call from Editor on startup
        void Shutdown();

        // quick helpers
        bool LoadScriptsDll(const std::string& dllPath);
        bool CallStart();                  // calls Scripts.Entry:Start()
        bool CallUpdate(float dt);         // calls Scripts.Entry:Update(float)

    private:
        MonoRuntime m_Mono;
        MonoAssembly* m_Scripts = nullptr;
        std::string m_ScriptsDir;
    };

} // namespace Boom
