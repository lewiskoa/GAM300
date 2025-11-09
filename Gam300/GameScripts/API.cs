// Boom/API.cs
using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;   // <-- ADD THIS

namespace Boom
{
    internal static class Native
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Boom_API_Log(string msg);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Boom_API_FindEntity(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Boom_API_GetPosition(ulong handle, out Vec3 pos);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Boom_API_SetPosition(ulong handle, ref Vec3 pos);

        
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Vec3 { public float X, Y, Z; }

    public static class API
    {
        public static void Log(string s) => Native.Boom_API_Log(s);
        public static ulong FindEntity(string name) => Native.Boom_API_FindEntity(name);
    }

    public static class Transform
    {
        public static Vec3 GetPosition(ulong handle)
        {
            Native.Boom_API_GetPosition(handle, out var p);
            return p;
        }

        public static void SetPosition(ulong handle, Vec3 p)
        {
            Native.Boom_API_SetPosition(handle, ref p);
        }
    }
}
