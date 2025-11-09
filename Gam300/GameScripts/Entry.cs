using System.Numerics;
using Boom;

namespace GameScripts
{
    public static class Entry
    {
        // Cache the entity once on Start
        private static ulong _player;
        private static float _speed = 5.0f; // units per second

        public static void Start()
        {
            // Match this to your scene entity name (e.g., "Player" or "Sphere")
            _player = API.FindEntity("Player");
            API.Log(_player != 0
                ? "[C#] Player entity found."
                : "[C#] Player entity NOT found. Rename an entity to 'Player'.");
        }

        public static void Update(float dt)
        {
            if (_player == 0) return; // nothing to do

            var pos = API.GetPosition(_player);

            // Build input vector from arrow keys
            float dx = 0, dz = 0;
            if (API.IsKeyDown(API.KEY_LEFT)) dx -= 1;
            if (API.IsKeyDown(API.KEY_RIGHT)) dx += 1;
            if (API.IsKeyDown(API.KEY_UP)) dz -= 1;
            if (API.IsKeyDown(API.KEY_DOWN)) dz += 1;

            if (dx != 0 || dz != 0)
            {
                // Normalize diagonal to consistent speed
                float len = (float)System.Math.Sqrt(dx * dx + dz * dz);
                dx /= len; dz /= len;

                pos.X += dx * _speed * dt;
                pos.Z += dz * _speed * dt;

                API.SetPosition(_player, pos);
            }
        }
    }
}
