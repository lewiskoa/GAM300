#include "Scripting/MonoRuntime.h" // BOOM_INFO/ERROR macros
#include "Core.h"
namespace Boom {

    bool MonoRuntime::Init(const char* domainName, const char* assembliesPath)
    {
        // Optional: tell Mono where to look for configs/assemblies
        if (assembliesPath && *assembliesPath)
            mono_set_assemblies_path(assembliesPath); // e.g., ".;Scripts"

        // If you copied "etc" next to Editor.exe, point Mono at it:
        mono_set_dirs(".", "etc"); // (libdir, etcdir) – libdir unused on Windows here

        m_RootDomain = mono_jit_init_version(domainName ? domainName : "BoomDomain", "v4.0.30319");
        if (!m_RootDomain) { 
#ifdef DEBUG
            BOOM_ERROR("Mono: mono_jit_init_version failed"); return false;

#endif // DEBUG
        }

        // Optional child domain for app scripts (lets you unload later)
        m_AppDomain = mono_domain_create_appdomain(const_cast<char*>("BoomApp"), nullptr);
        if (m_AppDomain) mono_domain_set(m_AppDomain, /*force*/false);
#ifdef DEBUG
        BOOM_INFO("[Mono] Initialized: {}", RuntimeInfo());

#endif // DEBUG

        return true;
    }

    void MonoRuntime::Shutdown()
    {
        if (m_AppDomain) {
            mono_domain_set(m_RootDomain, /*force*/false);
            mono_domain_unload(m_AppDomain);
            m_AppDomain = nullptr;
        }
        if (m_RootDomain) {
            mono_jit_cleanup(m_RootDomain);
            m_RootDomain = nullptr;
        }
        BOOM_INFO("[Mono] Shutdown complete");
    }

    MonoAssembly* MonoRuntime::LoadAssembly(const char* path)
    {
        if (!path) return nullptr;
        MonoAssembly* asmHandle = mono_domain_assembly_open(mono_domain_get(), path);
        if (!asmHandle) {

#ifdef DEBUG
            BOOM_ERROR("[Mono] Failed to load assembly: {}", path);
#endif // DEBUG

            return nullptr;
        }
        const MonoImage* img = mono_assembly_get_image(asmHandle);
        BOOM_INFO("[Mono] Loaded assembly: {} (image ok={})", path, img ? "true" : "false");

        // NEW: remember it
        m_LoadedAssemblies.push_back(asmHandle);

        return asmHandle;
    }

    bool MonoRuntime::InvokeStatic(const char* fullMethodDesc, void** args, int argCount)
    {

        (void)args;
        (void)argCount;

        if (!fullMethodDesc) return false;

        MonoMethodDesc* desc = mono_method_desc_new(fullMethodDesc, /*include_namespace*/ true);
        if (!desc) {
#ifdef _DEBUG
            BOOM_ERROR("[Mono] Bad method desc: {}", fullMethodDesc);
#endif
            return false;
        }

        MonoMethod* method = nullptr;

        // Try corlib first
        if (MonoImage* imgCorlib = mono_get_corlib()) {
            method = mono_method_desc_search_in_image(desc, imgCorlib);
        }

        // Then any loaded game assemblies
        if (!method) {
            for (MonoAssembly* a : m_LoadedAssemblies) {
                if (!a) continue;
                if (MonoImage* img = mono_assembly_get_image(a)) {
                    method = mono_method_desc_search_in_image(desc, img);
                    if (method) break;
                }
            }
        }

        mono_method_desc_free(desc);

        if (!method) {
#ifdef _DEBUG
            BOOM_ERROR("[Mono] Method not found: {}", fullMethodDesc);
#endif
            return false;
        }

        MonoObject* exc = nullptr;
        MonoObject* ret = mono_runtime_invoke(method, nullptr, args, &exc);
        if (exc) {
            MonoString* s = mono_object_to_string(exc, nullptr);
            char* cstr = mono_string_to_utf8(s);
#ifdef _DEBUG
            BOOM_ERROR("[Mono] Exception: {}", cstr ? cstr : "(null)");
#endif
            if (cstr) mono_free(cstr);
            return false;
        }
#ifdef _DEBUG
        BOOM_INFO("[Mono] Invoked: {}", fullMethodDesc);
#endif
        (void)ret;
        return true;
    }

    const char* MonoRuntime::RuntimeInfo() const
    {
        return mono_get_runtime_build_info(); // e.g., "Mono JIT compiler version 6.12..."
    }

} // namespace Boom
