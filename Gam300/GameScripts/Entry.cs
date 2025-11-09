using System;
using Boom;

namespace GameScripts
{
    public static class Entry
    {
        // --- Key codes (GLFW) ---
        private const int KEY_W = 87;  // GLFW_KEY_W
        private const int KEY_A = 65;  // GLFW_KEY_A
        private const int KEY_S = 83;  // GLFW_KEY_S
        private const int KEY_D = 68;  // GLFW_KEY_D
        private const int KEY_SPACE = 32;  // GLFW_KEY_SPACE

        // Entity & movement state
        private static ulong _player;
        private static float _speed = 5.0f;   // walk speed (units/s)
        private static float _vy = 0.0f;   // vertical velocity (units/s)
        private static float _gravity = -20.0f; // gravity accel (units/s^2)
        private static float _jumpSpeed = 8.0f;   // initial jump velocity (units/s)
        private static float _groundY = 0.0f;   // cached ground height
        private static bool _grounded = true;

        public static void Start()
        {
            _player = API.FindEntity("Samurai");  // rename to your entity name if needed
            API.Log(_player != 0 ? "[C#] Player entity found." : "[C#] Player entity NOT found.");

            if (_player != 0)
            {
                var p = API.GetPosition(_player);
                _groundY = p.Y; // treat current Y as ground level
            }
        }

        public static void Update(float dt)
        {
            if (_player == 0) return;

            var pos = API.GetPosition(_player);

            // ------- Horizontal movement (WASD on XZ plane) -------
            float dx = 0f, dz = 0f;
            if (API.IsKeyDown(KEY_A)) dx -= 1f;
            if (API.IsKeyDown(KEY_D)) dx += 1f;
            if (API.IsKeyDown(KEY_W)) dz -= 1f; // forward = -Z (match your world setup)
            if (API.IsKeyDown(KEY_S)) dz += 1f;

            if (dx != 0f || dz != 0f)
            {
                // Normalize so diagonals aren’t faster
                float len = (float)Math.Sqrt(dx * dx + dz * dz);
                dx /= len; dz /= len;

                pos.X += dx * _speed * dt;
                pos.Z += dz * _speed * dt;
            }

            // ------------- Jump + gravity on Y ----------------
            // Jump only if grounded
            if (_grounded && API.IsKeyDown(KEY_SPACE))
            {
                _vy = _jumpSpeed;
                _grounded = false;
            }

            // Integrate gravity
            _vy += _gravity * dt;
            pos.Y += _vy * dt;

            // Ground clamp
            if (pos.Y <= _groundY)
            {
                pos.Y = _groundY;
                _vy = 0f;
                _grounded = true;
            }

            API.SetPosition(_player, pos);
        }
    }
}
