//replica of the windows explorer for resources browsing
#pragma once

#include "Context/Context.h"

struct DirectoryWindow : IWidget {
	BOOM_INLINE DirectoryWindow(AppInterface* context)
		: IWidget{ context }
		, selected{}
	{
	}

	BOOM_INLINE void OnShow(AppInterface* context) override {
		if (ImGui::Begin(ICON_FA_FOLDER_OPEN "\tProject")) {
			int32_t colNo{ (int32_t)((ImGui::GetContentRegionAvail().x) / (ASSET_SIZE + ImGui::GetStyle().ItemSpacing.x)) };
			colNo = glm::max(1, colNo);

			ImGuiTableFlags flags{
				ImGuiTableFlags_SizingFixedSame |
				ImGuiTableFlags_NoHostExtendX
			};

			if (ImGui::BeginTable("", colNo, flags)) {

				ImGui::EndTable();
			}


			ImGui::End();
		}
	}

private:
	//audio, font, model, texture
	//to be converted to ImTextureID for imgui icons
	std::map <std::string, Texture2D> icons;
	AssetID selected;
};