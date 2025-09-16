#pragma once
#include "Context/Context.h"

struct ViewportWindow : IWidget
{
	BOOM_INLINE ViewportWindow(GuiContext* context): IWidget(context)
	{
		m_Frame = (ImTextureID)context->GetSceneFrame();
	}

	//BOOM_INLINE void OnShow(GuiContext* context) override
	//{
	//	if (ImGui::Begin(ICON_FA_IMAGE "\tViewport"))
	//	{
	//		ImGui::BeginChild("Frame");
	//		{
	//			// show scene frame
	//			const ImVec2 size = ImGui::GetWindowContentRegionMax();
	//			ImGui::Image(m_Frame, size, ImVec2(0, 1), ImVec2(1, 0));
	//		}
	//		ImGui::EndChild();
	//	}
	//	ImGui::End();
	//}

	//BOOM_INLINE void OnSelect(Entity entity) override
	//{
	//	// nothing done           
	//}

private:
	ImTextureID m_Frame;
	ImVec2 m_Viewport;
};