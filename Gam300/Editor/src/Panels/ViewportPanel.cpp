// ViewportPanel.cpp - CORRECTED (matches your working old editor)
#include "Panels/ViewportPanel.h"
#include "Editor.h"
#include "Context/Context.h"
#include "Context/DebugHelpers.h"
#include "Vendors/imgui/imgui.h"
#include <type_traits>
#include <cstdint>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifndef ICON_FA_IMAGE
#define ICON_FA_IMAGE ""
#endif

namespace EditorUI {

    ViewportPanel::ViewportPanel(Editor* owner)
        : m_Owner(owner)
    {
        m_App = static_cast<Boom::AppInterface*>(m_Owner);
        m_Ctx = m_App ? m_App->GetContext() : nullptr;
    }

    void ViewportPanel::Render() { OnShow(); }

    void ViewportPanel::OnShow()
    {
        if (!m_ShowViewport) return;

        if (ImGui::Begin(ICON_FA_IMAGE "\tViewport", &m_ShowViewport)) {
            // Get available viewport space (updates automatically on fullscreen/resize)
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();

            // Detect size changes (including fullscreen transitions)
            static ImVec2 lastSize = { 0, 0 };
            bool sizeChanged = (viewportSize.x != lastSize.x || viewportSize.y != lastSize.y);

            if (sizeChanged && viewportSize.x > 1.0f && viewportSize.y > 1.0f) {
                lastSize = viewportSize;
                BOOM_INFO("Viewport resized to {}x{}", (int)viewportSize.x, (int)viewportSize.y);
            }

            // Store the current size for the editor to query
            m_Viewport = viewportSize;

            // Get the frame texture from engine (already rendered in main loop)
            uint32_t frameTexture = QuerySceneFrame();

            // Only proceed if we have valid texture and size
            if (frameTexture > 0 && viewportSize.x > 1.0f && viewportSize.y > 1.0f) {

                // Display the engine's rendered frame (just like your old working code)
                ImGui::Image(
                    (ImTextureID)(uintptr_t)frameTexture,
                    viewportSize,
                    ImVec2(0, 1),  // UV top-left (flipped for OpenGL)
                    ImVec2(1, 0)   // UV bottom-right
                );

                // Store viewport rect for other systems (camera input, gizmos, etc.)
                ImVec2 itemMin = ImGui::GetItemRectMin();
                ImVec2 itemMax = ImGui::GetItemRectMax();

                // Tooltip for debug
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Engine Viewport - Scene render output");
                }

            }
            else {
                // Debug info when texture is invalid
                ImGui::Text("Frame Texture ID: %u", frameTexture);
                ImGui::Text("Viewport Size: %.0fx%.0f", viewportSize.x, viewportSize.y);
                ImGui::Text("Waiting for engine frame data...");

                // Draw placeholder
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 canvasPos = ImGui::GetCursorScreenPos();

                if (viewportSize.x > 50 && viewportSize.y > 50) {
                    drawList->AddRectFilled(
                        canvasPos,
                        ImVec2(canvasPos.x + viewportSize.x, canvasPos.y + viewportSize.y),
                        IM_COL32(64, 64, 64, 255)
                    );
                    drawList->AddText(
                        ImVec2(canvasPos.x + 10, canvasPos.y + 10),
                        IM_COL32(255, 255, 255, 255),
                        "Engine Viewport"
                    );
                }
            }
        }
        ImGui::End();
    }

    void ViewportPanel::OnSelect(std::uint32_t entity_id)
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

            if (isTexture) {
                GLint width, height;
                glBindTexture(GL_TEXTURE_2D, m_FrameId);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
                BOOM_INFO("Texture actual size: {}x{}", width, height);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
        DebugHelpers::ValidateFrameData(m_FrameId, "ViewportPanel::DebugViewportState");
        BOOM_INFO("=== End Debug State ===");
    }

    std::uint32_t ViewportPanel::QuerySceneFrame() const
    {
        if (m_App) {
            return static_cast<std::uint32_t>(m_App->GetSceneFrame());
        }

        if (m_Ctx && m_Ctx->renderer) {
            return static_cast<std::uint32_t>(m_Ctx->renderer->GetFrame());
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