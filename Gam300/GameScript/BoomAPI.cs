using System.Runtime.InteropServices;

namespace BoomDotNet
{
    public static class API
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct ScriptVec3 { public float x, y, z; }
        const string Dll = "BoomEngine"; // your native DLL name (no .dll)
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_log([MarshalAs(UnmanagedType.LPUTF8Str)] string msg);
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong script_create_entity();
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_destroy_entity(ulong e);
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_set_position(ulong e, ScriptVec3 p);
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern ScriptVec3 script_get_position(ulong e);
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_update_all(float dt);
    }
}
