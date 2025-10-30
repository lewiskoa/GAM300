#include "Panels/ViewportPanel.h"
#include "Editor.h"
#include "Context/Context.h"
#include "Context/DebugHelpers.h"
#include "Vendors/imgui/imgui.h"
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

        // Try to get AppInterface from the owner (if Editor derives from it)
        m_App = dynamic_cast<Boom::AppInterface*>(m_Owner);

        // Cache context if Editor exposes it
        if constexpr (requires(Editor * e) { e->GetContext(); }) {
            m_Ctx = m_Owner->GetContext();
        }
        DEBUG_POINTER(m_App, "AppInterface");
        DEBUG_POINTER(m_Ctx, "AppContext");

        // Initial frame id via AppInterface first, then fallback to renderer
        uint32_t frameId = QuerySceneFrame();
        DebugHelpers::ValidateFrameData(frameId, "ViewportPanel constructor");

        m_FrameId = frameId;
        m_Frame = (ImTextureID)(uintptr_t)frameId;

        BOOM_INFO("ViewportPanel::Constructor - Frame ID: {}, ImTextureID: {}",
            m_FrameId, (void*)m_Frame);
    }

    void ViewportPanel::Render() { OnShow(); }

    void ViewportPanel::OnShow()
    {
        DEBUG_DLL_BOUNDARY("ViewportPanel::OnShow");
        if (!m_ShowViewport) return;

        // Refresh frame each frame (renderer may recreate it)
        uint32_t newFrameId = QuerySceneFrame();
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

            if (m_Frame && contentRegion.x > 0.0f && contentRegion.y > 0.0f)
            {
                GLint currentTexture{}; glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);
                ImGui::Image(m_Frame, contentRegion, ImVec2(0, 1), ImVec2(1, 0));
                GLint newTexture{};     glGetIntegerv(GL_TEXTURE_BINDING_2D, &newTexture);
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

    // ---------------- helpers ----------------

    uint32_t ViewportPanel::QuerySceneFrame() const
    {
        // Best: go through AppInterface (your source of truth)
        if (m_App) {
            return static_cast<uint32_t>(m_App->GetSceneFrame());
        }

        // Fallback: directly via renderer if context is available
        if (m_Ctx && m_Ctx->renderer) {
            // your GraphicsRenderer exposes GetFrame()
            return static_cast<uint32_t>(m_Ctx->renderer->GetFrame());
        }

        return 0u;
    }

    double ViewportPanel::QueryDeltaTime() const
    {
        if (m_App) return m_App->GetDeltaTime();
        if (m_Ctx) return m_Ctx->DeltaTime;
        return 0.0;
    }

} // namespace EditorUI
