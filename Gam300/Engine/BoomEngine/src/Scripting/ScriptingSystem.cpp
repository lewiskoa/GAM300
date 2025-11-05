#include "Scripting/ScriptingSystem.h"
#include "core.h"

namespace Boom {

    bool ScriptingSystem::Init(const std::string& scriptsDir)
    {
        m_ScriptsDir = scriptsDir;
        if (!m_Mono.Init("BoomDomain", scriptsDir.c_str())) return false;

        BOOM_INFO("[Scripting] Mono ready. {}", m_Mono.RuntimeInfo());
        return true;
    }

    void ScriptingSystem::Shutdown()
    {
        m_Scripts = nullptr;
        m_Mono.Shutdown();
    }

    bool ScriptingSystem::LoadScriptsDll(const std::string& dllPath)
    {
        m_Scripts = m_Mono.LoadAssembly(dllPath.c_str());
        return (m_Scripts != nullptr);
    }

    bool ScriptingSystem::CallStart()
    {
        // Fully-qualified static method: Namespace.Type:Method(signature)
        return m_Mono.InvokeStatic("Scripts.Entry:Start()");
    }

    bool ScriptingSystem::CallUpdate(float dt)
    {
        void* args[1];
        args[0] = &dt; // Mono expects float* for single-precision
        return m_Mono.InvokeStatic("Scripts.Entry:Update(single)", args, 1);
    }

} // namespace Boom
