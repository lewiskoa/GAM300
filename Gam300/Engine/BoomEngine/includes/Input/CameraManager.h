// CameraManager.h
#pragma once


#include "AppWindow.h"          // AppWindow, SetFOV(), IsMouseInCameraRegion()

namespace Boom {
    class CameraController {
    public:
        struct Config {
            float mouseSensitivityX = 0.25f;
            float mouseSensitivityY = 0.25;
            float multiplierStep = 0.01f;
            float minFov = CONSTANTS::MIN_FOV;
            float maxFov = CONSTANTS::MAX_FOV;
            bool  gateToViewportRect = true;
            bool  gateToRMB = true;
            bool  normalizeDiagonal = true;
            bool  clampPitch = true;
            float minPitchDeg = -89.f;
            float maxPitchDeg = 89.f;
        };

        explicit CameraController(AppWindow* window, Config cfg = {})
            : m_app(window), m_cfg(cfg) {
        }

        BOOM_INLINE void update(float /*dt*/) {
            if (!m_app) return;

            auto& input = m_app->input;
            const auto& s = input.current();

            const bool middleMouse = s.Mouse.test(GLFW_MOUSE_BUTTON_MIDDLE);
            const bool inRegion = !m_cfg.gateToViewportRect || m_app->IsMouseInCameraRegion(m_app->Handle().get());
            const bool rmb = !m_cfg.gateToRMB || s.Mouse.test(GLFW_MOUSE_BUTTON_RIGHT);
            const bool canUse = m_app->camInputEnabled && inRegion && rmb;          // look + WASD
            const bool canPan = m_app->camInputEnabled && inRegion && middleMouse;  // MMB pan

            // Read once per frame
            const glm::vec2 md = input.mouseDeltaLast();

            // WASD movement vector (camera-local intent)
            glm::vec3 movements{
                input.axis(GLFW_KEY_A, GLFW_KEY_D),
                input.axis(GLFW_KEY_Q, GLFW_KEY_E),
                input.axis(GLFW_KEY_S, GLFW_KEY_W)
            };
            if (m_cfg.normalizeDiagonal && glm::length2(movements) > 1e-6f)
                movements = glm::normalize(movements);

            const bool  running = s.Keys.test(GLFW_KEY_LEFT_SHIFT) || s.Keys.test(GLFW_KEY_RIGHT_SHIFT);
            const float base = CONSTANTS::CAM_PAN_SPEED * m_app->camMoveMultiplier;
            const float spd = base * (running ? CONSTANTS::CAM_RUN_MULTIPLIER : 1.f);

            // --- Give pan priority over look ---
            if (canPan) {
                // Pan in camera local X/Y using the one md we captured
                const glm::vec2 pan{ -md.x / float(m_app->getWidth()), md.y / float(m_app->getHeight()) };
                glm::vec3 localMove{
                    pan.x * m_app->camFOV * 0.09f,  // right
                    pan.y * m_app->camFOV * 0.10f,  // up
                    0.0f
                };
                m_app->camMoveDir = localMove * base;   // (or * spd if you want SHIFT to speed up pan)
            }
            else if (canUse) {
                // Look (RMB) using the same md
                m_app->camRot.y -= md.x *( m_cfg.mouseSensitivityX*CONSTANTS::CAM_PAN_SPEED);
                m_app->camRot.x -= md.y *( m_cfg.mouseSensitivityY*CONSTANTS::CAM_PAN_SPEED);
                if (m_cfg.clampPitch)
                    m_app->camRot.x = std::clamp(m_app->camRot.x, m_cfg.minPitchDeg, m_cfg.maxPitchDeg);

                // Move (WASD) in camera-local forward/right/up
                const float yaw = m_cfg.mouseSensitivityX / float(m_app->getWidth());   // deg per pixel
                const float pitch = m_cfg.mouseSensitivityY / float(m_app->getHeight());  // deg per pixel
                const float sy = std::sinf(yaw), cy = std::cosf(yaw);
                const float sp = std::sinf(pitch), cp = std::cosf(pitch);

                const glm::vec3 forward{ cp * sy, sp, cp * cy };
                const glm::vec3 worldUp{ 0,1,0 };
                const glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
                const glm::vec3 up = glm::normalize(glm::cross(right, forward));

                glm::vec3 moveWorld = right * movements.x + up * movements.y + forward * movements.z;
                if (glm::length2(moveWorld) > 1e-6f) moveWorld = glm::normalize(moveWorld);
                m_app->camMoveDir = moveWorld * spd;
            }
            else {
                // Only clear when neither panning nor using look/move
                m_app->camMoveDir = {};
            }

            // Scroll: speed multiplier and FOV
            const glm::vec2 sd = input.scrollDelta();
            if (rmb && inRegion) {
                m_app->camMoveMultiplier = std::clamp(m_app->camMoveMultiplier + sd.y * m_cfg.multiplierStep, 0.01f, 100.0f);
            }
            if (inRegion) {
                m_app->SetFOV(std::clamp(m_app->camFOV - sd.y, m_cfg.minFov, m_cfg.maxFov));
            }
        }


    private:
        AppWindow* m_app = nullptr;
        Config     m_cfg{};
    };
}