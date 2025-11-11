#pragma once


#include "Core.h"                  // for GLFW, Boom basics
#include "AppWindow.h"             // Boom::AppWindow (from engine)
#include "common/Events.h"         // your event types if needed

// ImGui (editor-only)
#include "Vendors/imgui/imgui.h"
#include "Vendors/imgui/backends/imgui_impl_glfw.h"

struct GLFWwindow;

namespace EditorUI
{
    using namespace Boom;

    // Helper: the engine sets glfwSetWindowUserPointer(window, AppWindow*)
    static AppWindow* GetAppWindow(GLFWwindow* window)
    {
        return static_cast<AppWindow*>(glfwGetWindowUserPointer(window));
    }

    // ---------- Callbacks ----------

    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

        AppWindow* self = GetAppWindow(window);
        if (!self)
            return;

        // Let ESC close window regardless of ImGui focus (optional)
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            return;
        }

        // If ImGui wants keyboard (text box, menus etc), don't send to game
        if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard)
            return;

        auto& input = self->GetInputSystem();
        input.onKey(key, scancode, action, mods);

        if (auto* disp = self->GetDispatcher())
        {
            switch (action)
            {
            case GLFW_PRESS:   disp->PostEvent<KeyPressEvent>(key);   break;
            case GLFW_RELEASE: disp->PostEvent<KeyReleaseEvent>(key); break;
            case GLFW_REPEAT:  disp->PostEvent<KeyRepeatEvent>(key);  break;
            }
        }
    }

    static void CharCallback(GLFWwindow* window, unsigned int c)
    {
        // Needed for ImGui::InputText
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_CharCallback(window, c);

        // If ImGui wants keyboard, we don't forward chars to engine.
        if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard)
            return;

        // If you add your own console / text input later, handle it here.
    }

    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

        AppWindow* self = GetAppWindow(window);
        if (!self)
            return;

        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::GetCurrentContext() && io.WantCaptureMouse)
            return;

        auto& input = self->GetInputSystem();
        input.onMouseButton(button, action, mods);

        if (auto* disp = self->GetDispatcher())
        {
            if (action == GLFW_PRESS)   disp->PostEvent<MouseDownEvent>(button);
            if (action == GLFW_RELEASE) disp->PostEvent<MouseReleaseEvent>(button);
        }
    }

    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

        AppWindow* self = GetAppWindow(window);
        if (!self)
            return;

        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::GetCurrentContext() && io.WantCaptureMouse)
            return;

        auto& input = self->GetInputSystem();
        input.onScroll(xoffset, yoffset);

        if (auto* disp = self->GetDispatcher())
            disp->PostEvent<MouseWheelEvent>(xoffset, yoffset);
    }

    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

        AppWindow* self = GetAppWindow(window);
        if (!self)
            return;

        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::GetCurrentContext() && io.WantCaptureMouse)
            return;

        auto& input = self->GetInputSystem();
        input.onCursorPos(xpos, ypos);

        if (auto* disp = self->GetDispatcher())
        {
            disp->PostEvent<MouseMotionEvent>(xpos, ypos);

            if (input.current().Mouse.any())
            {
                // If you track precise mouse deltas, plug them here.
                disp->PostEvent<MouseDragEvent>(0.0, 0.0);
            }
        }
    }

    // ---------- Install from Editor main ----------

    BOOM_INLINE void InstallEditorInputCallbacks(GLFWwindow* window)
    {
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetCharCallback(window, CharCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
        glfwSetScrollCallback(window, ScrollCallback);
        glfwSetCursorPosCallback(window, CursorPosCallback);
    }
}
