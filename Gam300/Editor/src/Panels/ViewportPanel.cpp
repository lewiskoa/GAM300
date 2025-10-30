#include "Panels/ViewportPanel.h"

// Pull full types here (keeps header lean)
#include "Editor.h"
#include "Context/Context.h"
#include "Context/DebugHelpers.h"

// ImGui
#include "Vendors/imgui/imgui.h"

// GL (ensure your project initializes GLEW/loader once)
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifndef ICON_FA_IMAGE
#define ICON_FA_IMAGE ""
#endif

namespace EditorUI {

    ViewportPanel::ViewportPanel(Editor* owner)
        : m_Owner(owner)
    {
        DEBUG_DLL_BOUNDARY("ViewportPanel::Constructor");

        if (!m_Owner) {
            BOOM_ERROR("ViewportPanel::Constructor - Null owner!");
            return;
        }

        // Editor must expose: Boom::AppContext* GetContext() const;
        m_Ctx = m_Owner->GetContext();
        DEBUG_POINTER(m_Ctx, "AppContext");

        if (!m_Ctx) {
            BOOM_ERROR("ViewportPanel::Constructor - Null AppContext!");
            return;
        }

        // Grab initial scene frame
        uint32_t frameId = m_Ctx->GetSceneFrame();
        DebugHelpers::ValidateFrameData(frameId, "ViewportPanel constructor");

        m_FrameId = frameId;
        m_Frame = (ImTextureID)(uintptr_t)frameId;

        BOOM_INFO("ViewportPanel::Constructor - Frame ID: {}, ImTextureID: {}",
            m_FrameId, (void*)m_Frame);
    }

    void ViewportPanel::Render()
    {
        OnShow();
    }

    void ViewportPanel::OnShow()
    {
        DEBUG_DLL_BOUNDARY("ViewportPanel::OnShow");

        if (!m_ShowViewport) return;

        if (!m_Ctx) {
            BOOM_ERROR("ViewportPanel::OnShow - Null AppContext!");
            return;
        }

        // Refresh frame id each frame (renderer may recreate it)
        uint32_t newFrameId = m_Ctx->GetSceneFrame();
        if (newFrameId != m_FrameId) {
            BOOM_INFO("ViewportPanel::OnShow - Frame ID changed: {} -> {}", m_FrameId, newFrameId);
            m_FrameId = newFrameId;
            m_Frame = (ImTextureID)(uintptr_t)newFrameId;
        }

        DebugHelpers::ValidateFrameData(m_FrameId, "ViewportPanel::OnShow");

        if (ImGui::Begin(ICON_FA_IMAGE "\tViewport", &m_ShowViewport))
        {
            ImVec2 contentRegion = ImGui::GetContentRegionAvail();
            m_Viewport = contentRegion;

            // Optional: very chatty; comment out if noisy
            // BOOM_INFO("ViewportPanel::OnShow - Viewport size: {}x{}", contentRegion.x, contentRegion.y);

            if (m_Frame && contentRegion.x > 0.0f && contentRegion.y > 0.0f)
            {
                GLint currentTexture{};
                glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);

                // Draw scene texture; OpenGL origin flip (v: [1..0])
                ImGui::Image(m_Frame, contentRegion, ImVec2(0, 1), ImVec2(1, 0));

                // Hover hints / input routing can go here
                // if (ImGui::IsItemHovered()) { ... }

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

    void ViewportPanel::OnSelect(uint32_t entity_id)
    {
        DEBUG_DLL_BOUNDARY("ViewportPanel::OnSelect");
        BOOM_INFO("ViewportPanel::OnSelect - Entity selected: {}", entity_id);
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

} // namespace EditorUI
