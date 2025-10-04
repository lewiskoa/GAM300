using System.Runtime.InteropServices;

namespace BoomDotNet
{
    public static class API
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct ScriptVec3 { public float x, y, z; }

        const string Dll = "BoomEngine";

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_log([MarshalAs(UnmanagedType.LPUTF8Str)] string msg);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern uint script_create_entity();                   // CHANGED: uint

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_destroy_entity(uint e);            // CHANGED: uint

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_set_position(uint e, ScriptVec3 p); // CHANGED: uint

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern ScriptVec3 script_get_position(uint e);       // CHANGED: uint

        // -------- Physics --------
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_physics_set_gravity(ScriptVec3 g);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_add_rigidbody(uint e, float mass);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_add_box_collider(uint e, ScriptVec3 halfExtents);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_add_sphere_collider(uint e, float radius);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_set_linear_velocity(uint e, ScriptVec3 v);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern ScriptVec3 script_get_linear_velocity(uint e);

        // Optional if scripting may tick PhysX
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_physics_step(float dt);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_add_model(uint e, uint modelId, uint materialId);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void script_set_scale(uint e, ScriptVec3 s);
    }
}