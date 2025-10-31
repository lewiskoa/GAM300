#pragma once
#include "Widgets.h"
#include "DebugHelpers.h"
#include "Audio/Audio.hpp"

#include <memory>
#include <vector>
#include <type_traits>

// Forward declarations (avoid pulling heavy headers here)
struct ImGuiContext;          // ImGui context is global-namespace
struct GLFWwindow;            // from GLFW

// ============================= GuiContext =============================
struct GuiContext : AppInterface
{
    // Lifecycle
    virtual ~GuiContext() override;
    BOOM_INLINE virtual void OnGuiStart() {}
    BOOM_INLINE virtual void OnGuiFrame() {}
    virtual void OnStart() override final;
    virtual void OnUpdate() override final;

    // Widget helpers (kept inline; no backend dependency)
    template<typename T, typename... Args>
    BOOM_INLINE void AttachWindow(Args&&... args)
    {
        BOOM_STATIC_ASSERT(std::is_base_of<IWidget, T>::value);
        auto window = std::make_unique<T>(this, std::forward<Args>(args)...);
        BOOM_INFO("GuiContext::AttachWindow - Created window: {}", (void*)window.get());
        m_Windows.push_back(std::move(window));
    }

    template<typename T, typename... Args>
    BOOM_INLINE auto CreateWidget(Args&&... args)
    {
        BOOM_STATIC_ASSERT(std::is_base_of<IWidget, T>::value);
        auto widget = std::make_unique<T>(this, std::forward<Args>(args)...);
        BOOM_INFO("GuiContext::CreateWidget - Created widget: {}", (void*)widget.get());
        return widget;
    }

private:
    // Implement these in a .cpp (they touch GLFW/ImGui backends)
    bool EnsureContextCurrent(GLFWwindow* window);
    void LoadFonts();

private:
    std::vector<std::unique_ptr<IWidget>> m_Windows;
    std::shared_ptr<GLFWwindow> m_EngineWindow = nullptr;
};

// ========================= GuiContextNoSwitch =========================
struct GuiContextNoSwitch : AppInterface
{
    virtual ~GuiContextNoSwitch() override;

    // One-time init that adopts an already-current context
    void InitializeWithExistingContext(GLFWwindow* window);

    // Per-frame
    virtual void OnUpdate() override final;

    // Widget helper (inline ok)
    template<typename T, typename... Args>
    BOOM_INLINE void AttachWindow(Args&&... args)
    {
        auto window = std::make_unique<T>(this, std::forward<Args>(args)...);
        m_Windows.push_back(std::move(window));
    }

private:
    // Implement in .cpp
    void LoadFonts();

private:
    std::vector<std::unique_ptr<IWidget>> m_Windows;
    GLFWwindow* m_Window = nullptr;
};
