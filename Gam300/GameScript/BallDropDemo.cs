using BoomDotNet;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameScripts
{
    // This will be discovered and run by Entry.Init (it looks for IBehaviour classes)
    public sealed class BallDropDemo : IBehaviour
    {
        // -- Tunables --
        const int BallCount = 40;
        const float BallRadius = 0.5f;
        const float BallMass = 1.0f;
        const float SpawnHeight = 10.0f;
        const float SpawnSpreadXZ = 8.0f;

        // If your engine does NOT step physics itself, set this true to call script_physics_step(dt)
        const bool StepPhysicsFromScript = false;

        readonly Random _rng = new Random(1234);

        static API.ScriptVec3 V3(float x, float y, float z) => new API.ScriptVec3 { x = x, y = y, z = z };

        public void OnStart()
        {
            // 1) Gravity
            API.script_physics_set_gravity(V3(0, -9.81f, 0));

            // 2) Ground: large thin box, static rigidbody (mass = 0)
            {
                uint ground = API.script_create_entity();
                API.script_set_position(ground, V3(0, 0, 0));
                API.script_add_box_collider(ground, V3(20f, 0.25f, 20f)); // half-extents
                API.script_add_rigidbody(ground, 0.0f);                   // 0 => static
                API.script_log("Created ground plane");
            }

            // 3) Spheres: dynamic rigidbodies with sphere colliders
            for (int i = 0; i < BallCount; ++i)
            {
                float x = (float)((_rng.NextDouble() * 2 - 1) * SpawnSpreadXZ);
                float z = (float)((_rng.NextDouble() * 2 - 1) * SpawnSpreadXZ);
                float y = SpawnHeight + i * (BallRadius * 0.1f); // slight stagger to avoid perfect overlap

                uint e = API.script_create_entity();
                API.script_set_position(e, V3(x, y, z));
                API.script_add_sphere_collider(e, BallRadius);
                API.script_add_rigidbody(e, BallMass);

                // Optional: little random nudge so stacks don’t settle perfectly
                float jx = (float)((_rng.NextDouble() * 2 - 1) * 0.5);
                float jz = (float)((_rng.NextDouble() * 2 - 1) * 0.5);
                API.script_set_linear_velocity(e, V3(jx, 0, jz));
            }

            API.script_log($"Spawned {BallCount} balls.");
        }

        public void OnUpdate(float dt)
        {
            if (StepPhysicsFromScript)
                API.script_physics_step(dt);
        }
    }
}