#include "Scripting/MonoRuntime.h"
#include "core.h" // BOOM_INFO/ERROR macros

namespace Boom {

    bool MonoRuntime::Init(const char* domainName, const char* assembliesPath)
    {
        // Optional: tell Mono where to look for configs/assemblies
        if (assembliesPath && *assembliesPath)
            mono_set_assemblies_path(assembliesPath); // e.g., ".;Scripts"

        // If you copied "etc" next to Editor.exe, point Mono at it:
        mono_set_dirs(".", "etc"); // (libdir, etcdir) – libdir unused on Windows here

        m_RootDomain = mono_jit_init_version(domainName ? domainName : "BoomDomain", "v4.0.30319");
        if (!m_RootDomain) { BOOM_ERROR("Mono: mono_jit_init_version failed"); return false; }

        // Optional child domain for app scripts (lets you unload later)
        m_AppDomain = mono_domain_create_appdomain(const_cast<char*>("BoomApp"), nullptr);
        if (m_AppDomain) mono_domain_set(m_AppDomain, /*force*/false);

        BOOM_INFO("[Mono] Initialized: {}", RuntimeInfo());
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
            BOOM_ERROR("[Mono] Failed to load assembly: {}", path);
            return nullptr;
        }
        const MonoImage* img = mono_assembly_get_image(asmHandle);
        BOOM_INFO("[Mono] Loaded assembly: {} (image ok={})", path, img ? "true" : "false");
        return asmHandle;
    }

    bool MonoRuntime::InvokeStatic(const char* fullMethodDesc, void** args, int argCount)
    {
        if (!fullMethodDesc) return false;

        MonoMethodDesc* desc = mono_method_desc_new(fullMethodDesc, /*include_namespace*/ true);
        if (!desc) { BOOM_ERROR("[Mono] Bad method desc: {}", fullMethodDesc); return false; }

        // Search in all loaded images in current domain
        MonoImage* corlib = mono_get_corlib();
        MonoMethod* method = corlib ? mono_method_desc_search_in_image(desc, corlib) : nullptr;

        if (!method) {
            // Fallback: walk loaded assemblies
            bool found = false;
            MonoDomain* dom = mono_domain_get();
            MonoAssembly** asms = nullptr;
            int count = mono_domain_get_assemblies(dom, &asms);
            for (int i = 0; i < count && !found; ++i) {
                MonoImage* img = mono_assembly_get_image(asms[i]);
                method = mono_method_desc_search_in_image(desc, img);
                found = (method != nullptr);
            }
        }
        mono_method_desc_free(desc);

        if (!method) { BOOM_ERROR("[Mono] Method not found: {}", fullMethodDesc); return false; }

        MonoObject* exc = nullptr;
        MonoObject* ret = mono_runtime_invoke(method, nullptr, args, &exc);
        if (exc) {
            MonoString* s = mono_object_to_string(exc, nullptr);
            char* cstr = mono_string_to_utf8(s);
            BOOM_ERROR("[Mono] Exception: {}", cstr ? cstr : "(null)");
            if (cstr) mono_free(cstr);
            return false;
        }
        (void)ret;
        BOOM_INFO("[Mono] Invoked: {}", fullMethodDesc);
        return true;
    }

    const char* MonoRuntime::RuntimeInfo() const
    {
        return mono_get_runtime_build_info(); // e.g., "Mono JIT compiler version 6.12..."
    }

} // namespace Boom
