#pragma once
#include "Context/Context.h"
#include "Context/DebugHelpers.h"

struct ViewportWindow : IWidget
{
	BOOM_INLINE ViewportWindow(AppInterface* c) : IWidget(c)
	{
		DEBUG_DLL_BOUNDARY("ViewportWindow::Constructor");
		DEBUG_POINTER(context, "AppInterface");

		if (!context) {  // Check for null context
			BOOM_ERROR("ViewportWindow::Constructor - Null context!");
			m_Frame = 0;
			return;
		}

		// Get the scene frame with debugging
		uint32_t frameId = context->GetSceneFrame();
		DebugHelpers::ValidateFrameData(frameId, "ViewportWindow constructor");

		m_Frame = (ImTextureID)(uintptr_t)frameId;
		m_FrameId = frameId;

		BOOM_INFO("ViewportWindow::Constructor - Frame ID: {}, ImTextureID: {}",
			frameId, (void*)m_Frame);
	}

	BOOM_INLINE void OnShow() override
	{
		DEBUG_DLL_BOUNDARY("ViewportWindow::OnShow");

		if (!context) {
			BOOM_ERROR("ViewportWindow::OnShow - Null context!");
			return;
		}

		// Refresh frame data each time
		uint32_t newFrameId = context->GetSceneFrame();
		if (newFrameId != m_FrameId) {
			BOOM_INFO("ViewportWindow::OnShow - Frame ID changed: {} -> {}", m_FrameId, newFrameId);
			m_FrameId = newFrameId;
			m_Frame = (ImTextureID)(uintptr_t)newFrameId;
		}

		DebugHelpers::ValidateFrameData(m_FrameId, "ViewportWindow::OnShow");

		if (ImGui::Begin(ICON_FA_IMAGE "\tViewport"))
		{
			// Get current window size
			ImVec2 contentRegion = ImGui::GetContentRegionAvail();

			// Store viewport size for debugging
			m_Viewport = contentRegion;

			BOOM_INFO("ViewportWindow::OnShow - Viewport size: {}x{}",
				contentRegion.x, contentRegion.y);

			if (m_Frame && contentRegion.x > 0 && contentRegion.y > 0) {
				// Debug texture binding
				GLint currentTexture;
				glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);

				// Show scene frame with proper UV coordinates for OpenGL
				ImGui::Image(m_Frame, contentRegion, ImVec2(0, 1), ImVec2(1, 0));

				// Check for ImGui errors
				if (ImGui::IsItemHovered()) {
					BOOM_INFO("ViewportWindow::OnShow - Viewport is hovered");
				}

				// Restore texture binding if needed
				GLint newTexture;
				glGetIntegerv(GL_TEXTURE_BINDING_2D, &newTexture);
				if (currentTexture != newTexture) {
					BOOM_WARN("ViewportWindow::OnShow - Texture binding changed: {} -> {}",
						currentTexture, newTexture);
				}
			}
			else {
				// Show error message if frame is invalid
				ImGui::TextColored(ImVec4(1, 0, 0, 1), "Invalid frame data!");
				ImGui::Text("Frame ID: %u", m_FrameId);
				ImGui::Text("Frame Ptr: %p", m_Frame);
				ImGui::Text("Content Region: %.1fx%.1f", contentRegion.x, contentRegion.y);
			}
		}
		ImGui::End();
	}

	BOOM_INLINE void OnSelect(Entity entity) override
	{
		DEBUG_DLL_BOUNDARY("ViewportWindow::OnSelect");
		BOOM_INFO("ViewportWindow::OnSelect - Entity selected: {}", (uint32_t)entity);
		// Add selection logic here if needed
	}

	// Debug function to get current viewport state
	BOOM_INLINE void DebugViewportState() const
	{
		BOOM_INFO("=== ViewportWindow Debug State ===");
		BOOM_INFO("Frame ID: {}", m_FrameId);
		BOOM_INFO("Frame Ptr: {}", (void*)m_Frame);
		BOOM_INFO("Viewport Size: {}x{}", m_Viewport.x, m_Viewport.y);

		if (m_FrameId != 0) {
			GLboolean isTexture = glIsTexture(m_FrameId);
			BOOM_INFO("Frame is valid OpenGL texture: {}", isTexture);
		}

		DebugHelpers::ValidateFrameData(m_FrameId, "DebugViewportState");
		BOOM_INFO("=== End Debug State ===");
	}

private:
	ImTextureID m_Frame;
	uint32_t m_FrameId;
	ImVec2 m_Viewport;
};