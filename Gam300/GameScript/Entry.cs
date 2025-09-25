using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BoomDotNet
{
    public static class Entry
    {
        private static readonly System.Collections.Generic.List<IBehaviour> _updates = new();

        [UnmanagedCallersOnly(EntryPoint = "Init", CallConvs = new[] { typeof(CallConvCdecl) })]
        public static int Init(IntPtr _)
        {
            API.script_log("C# Init OK");

            var dll = Path.GetFullPath(@"GameScripts\bin\Debug\net8.0\GameScripts.dll");
            if (File.Exists(dll))
            {
                var asm = Assembly.LoadFrom(dll);
                foreach (var t in asm.GetTypes().Where(t => typeof(IBehaviour).IsAssignableFrom(t) && t.IsClass && !t.IsAbstract))
                {
                    if (Activator.CreateInstance(t) is IBehaviour b)
                    {
                        b.OnStart();
                        _updates.Add(b);
                    }
                }
                API.script_log($"Loaded {_updates.Count} behaviour(s) from GameScripts.dll");
            }
            else
            {
                API.script_log("GameScripts.dll not found yet.");
            }
            return 0;
        }

        // exported directly (no delegate)
        [UnmanagedCallersOnly(EntryPoint = "Tick", CallConvs = new[] { typeof(CallConvCdecl) })]
        public static void Tick(float dt)
        {
            foreach (var b in _updates)
                b.OnUpdate(dt);
        }
    }
}
