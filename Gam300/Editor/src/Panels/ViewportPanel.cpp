// ViewportPanel.cpp - WITH FULLSCREEN SUPPORT
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
        , m_IsFullscreen(false)
    {
        m_App = static_cast<Boom::AppInterface*>(m_Owner);
        m_Ctx = m_App ? owner->GetContext() : nullptr;
    }

    void ViewportPanel::Render() { OnShow(); }

    void ViewportPanel::OnShow()
    {
        if (!m_ShowViewport) return;

        // Handle fullscreen toggle (F11 key)
        if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
            m_IsFullscreen = !m_IsFullscreen;
        }

        // Window flags for fullscreen mode
        ImGuiWindowFlags windowFlags = 0;
        if (m_IsFullscreen) {
            windowFlags |= ImGuiWindowFlags_NoTitleBar;
            windowFlags |= ImGuiWindowFlags_NoCollapse;
            windowFlags |= ImGuiWindowFlags_NoResize;
            windowFlags |= ImGuiWindowFlags_NoMove;
            windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
            windowFlags |= ImGuiWindowFlags_NoNavFocus;
            windowFlags |= ImGuiWindowFlags_NoBackground;

            // Set window to cover the entire viewport
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
        }

        if (ImGui::Begin(ICON_FA_IMAGE "\tViewport", &m_ShowViewport, windowFlags))
        {
            // Add fullscreen toggle button in the top-right corner
            if (!m_IsFullscreen) {
                ImGui::SameLine(ImGui::GetWindowWidth() - 80);
                if (ImGui::Button("Fullscreen")) {
                    m_IsFullscreen = true;
                }
            }
            else {
                // Show exit fullscreen button
                ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 120, 10));
                if (ImGui::Button("Exit Fullscreen") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    m_IsFullscreen = false;
                }
            }

            // 1) Get available space & aspect
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();
            m_Viewport = viewportSize;
            const float aspect = (viewportSize.y > 1.0f) ? (viewportSize.x / viewportSize.y) : 1.0f;

            // 2) Get the frame texture from the engine
            const uint32_t frameTexture = QuerySceneFrame();

            if (frameTexture > 0 && viewportSize.x > 1.0f && viewportSize.y > 1.0f)
            {
                // 3) Draw the backbuffer/scene image
                ImGui::Image(
                    (ImTextureID)(uintptr_t)frameTexture,
                    viewportSize,
                    ImVec2(0, 1),  // flip for OpenGL
                    ImVec2(1, 0)
                );

                // 4) Determine the viewport rect in ImGui space
                const ImVec2 itemMin = ImGui::GetItemRectMin();
                const ImVec2 itemMax = ImGui::GetItemRectMax();
                const ImVec2 rectSz = ImVec2(itemMax.x - itemMin.x, itemMax.y - itemMin.y);

                // 5) Hover/focus: only allow camera input when actually over this image
                const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
                const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && hovered;

                // 6) Convert to GLFW client-space and inform the engine window
                const ImVec2 mainPos = ImGui::GetMainViewport()->Pos;
                const double localX = double(itemMin.x - mainPos.x);
                const double localY = double(itemMin.y - mainPos.y);
                const double localW = double(rectSz.x);
                const double localH = double(rectSz.y);

                if (m_Ctx && m_Ctx->window)
                    m_Ctx->window->SetCameraInputRegion(localX, localY, localW, localH, /*allow*/ hovered && focused);

                // 7) Build active camera view/projection with the current aspect
                if (m_Ctx)
                {
                    auto camView = m_Ctx->scene.view<Boom::CameraComponent, Boom::TransformComponent>();
                    if (camView.begin() != camView.end())
                    {
                        auto eid = *camView.begin();
                        auto& camComp = camView.get<Boom::CameraComponent>(eid);
                        auto& trans = camView.get<Boom::TransformComponent>(eid);

                        const glm::mat4 view = camComp.camera.View(trans.transform);
                        const glm::mat4 proj = camComp.camera.Projection(aspect);
                        (void)view; (void)proj;
                    }
                }

                // Tooltip
                if (hovered && !m_IsFullscreen)
                    ImGui::SetTooltip("Engine Viewport - Scene render output\nPress F11 for fullscreen");
            }
            else
            {
                // Fallback UI when no frame is available
                ImGui::Text("Frame Texture ID: %u", frameTexture);
                ImGui::Text("Viewport Size: %.0fx%.0f", viewportSize.x, viewportSize.y);
                ImGui::Text("Waiting for engine frame data...");

                if (viewportSize.x > 50 && viewportSize.y > 50) {
                    ImDrawList* drawList = ImGui::GetWindowDrawList();
                    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
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