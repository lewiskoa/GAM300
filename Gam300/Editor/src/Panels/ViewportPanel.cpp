#include "Panels/ViewportPanel.h"

// NOTE: Include your GL header that provides glGetIntegerv/glIsTexture.
// If you use GLAD:   #include <glad/glad.h>
// If you use GLEW:   #include <GL/glew.h>
// If already included in your PCH, you can omit this include here.
#include <GLFW/glfw3.h> 

ViewportPanel::ViewportPanel(AppInterface* ctx)
    : IWidget(ctx)
{
    DEBUG_DLL_BOUNDARY("ViewportPanel::Constructor");
    DEBUG_POINTER(context, "AppInterface");

    if (!context) {
        BOOM_ERROR("ViewportPanel::Constructor - Null context!");
        m_Frame = 0;
        return;
    }

    // Get initial scene frame
    uint32_t frameId = context->GetSceneFrame();
    DebugHelpers::ValidateFrameData(frameId, "ViewportPanel constructor");

    m_Frame = (ImTextureID)(uintptr_t)frameId;
    m_FrameId = frameId;

    BOOM_INFO("ViewportPanel::Constructor - Frame ID: {}, ImTextureID: {}", frameId, (void*)m_Frame);
}

void ViewportPanel::Render()
{
    OnShow();
}

void ViewportPanel::OnShow()
{
    DEBUG_DLL_BOUNDARY("ViewportPanel::OnShow");

    if (!m_ShowViewport) return;

    if (!context) {
        BOOM_ERROR("ViewportPanel::OnShow - Null context!");
        return;
    }

    // Refresh frame id each frame (in case the renderer recreated it)
    uint32_t newFrameId = context->GetSceneFrame();
    if (newFrameId != m_FrameId) {
        BOOM_INFO("ViewportPanel::OnShow - Frame ID changed: {} -> {}", m_FrameId, newFrameId);
        m_FrameId = newFrameId;
        m_Frame = (ImTextureID)(uintptr_t)newFrameId;
    }

    DebugHelpers::ValidateFrameData(m_FrameId, "ViewportPanel::OnShow");

    if (ImGui::Begin(ICON_FA_IMAGE "\tViewport", &m_ShowViewport))
    {
        // Available draw area for the image
        ImVec2 contentRegion = ImGui::GetContentRegionAvail();
        m_Viewport = contentRegion;

        BOOM_INFO("ViewportPanel::OnShow - Viewport size: {}x{}", contentRegion.x, contentRegion.y);

        if (m_Frame && contentRegion.x > 0.0f && contentRegion.y > 0.0f)
        {
            // Snapshot current binding to detect if our draw changed it
            GLint currentTexture{};
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);

            // Draw scene texture; OpenGL origin flip (v: [1..0])
            ImGui::Image(m_Frame, contentRegion, ImVec2(0, 1), ImVec2(1, 0));

            if (ImGui::IsItemHovered()) {
                BOOM_INFO("ViewportPanel::OnShow - Viewport is hovered");
            }

            GLint newTexture{};
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &newTexture);
            if (currentTexture != newTexture) {
                BOOM_WARN("ViewportPanel::OnShow - Texture binding changed: {} -> {}", currentTexture, newTexture);
            }
        }
        else
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Invalid frame data!");
            ImGui::Text("Frame ID: %u", m_FrameId);
            ImGui::Text("Frame Ptr: %p", m_Frame);
            ImGui::Text("Content Region: %.1fx%.1f", contentRegion.x, contentRegion.y);
        }
    }
    ImGui::End();
}

void ViewportPanel::OnSelect(Entity entity)
{
    DEBUG_DLL_BOUNDARY("ViewportPanel::OnSelect");
    BOOM_INFO("ViewportPanel::OnSelect - Entity selected: {}", (uint32_t)entity);
    // Hook selection-dependent viewport behaviors here (gizmos, focus, etc.)
}

void ViewportPanel::DebugViewportState() const
{
    BOOM_INFO("=== ViewportPanel Debug State ===");
    BOOM_INFO("Frame ID: {}", m_FrameId);
    BOOM_INFO("Frame Ptr: {}", (void*)m_Frame);
    BOOM_INFO("Viewport Size: {}x{}", m_Viewport.x, m_Viewport.y);

    if (m_FrameId != 0) {
        GLboolean isTexture = glIsTexture(m_FrameId);
        BOOM_INFO("Frame is valid OpenGL texture: {}", isTexture);
    }

    DebugHelpers::ValidateFrameData(m_FrameId, "ViewportPanel::DebugViewportState");
    BOOM_INFO("=== End Debug State ===");
}
