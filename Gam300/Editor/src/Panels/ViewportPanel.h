#pragma once
#include "Context/Context.h"
#include "Context/DebugHelpers.h"
#include "Vendors/imgui/imgui.h"

// Fallback if your FontAwesome alias isn't defined in your global headers
#ifndef ICON_FA_IMAGE
#define ICON_FA_IMAGE ""
#endif

// ViewportPanel: draws the scene framebuffer texture inside an ImGui window.
class ViewportPanel : public IWidget
{
public:
    BOOM_INLINE explicit ViewportPanel(AppInterface* ctx);

    // Call this each frame from your editor render pass
    void Render();               // wrapper -> calls OnShow()
    BOOM_INLINE void OnShow() override;
    BOOM_INLINE void OnSelect(Entity entity) override;

    // Optional visibility control
    BOOM_INLINE void Show(bool v) { m_ShowViewport = v; }
    BOOM_INLINE bool IsVisible() const { return m_ShowViewport; }

    // Debug helper
    BOOM_INLINE void DebugViewportState() const;

private:
    bool       m_ShowViewport = true;

    ImTextureID m_Frame = nullptr;
    uint32_t    m_FrameId = 0;
    ImVec2      m_Viewport{ 0.0f, 0.0f };
};
