#pragma once
#include "Context/Context.h"

struct ResourceWindow : IWidget {
	BOOM_INLINE ResourceWindow(AppInterface* context) 
		: IWidget{ context }
		, iconImage{"Icons/asset.png"}
		, icon{ (ImTextureID)iconImage }
	{}

	BOOM_INLINE void OnShow(AppInterface* context) override {
		if (ImGui::Begin(ICON_FA_FOLDER_OPEN "\tResources")) {
			int32_t colNo{ (int32_t)(ImGui::GetContentRegionAvail().x / ASSET_SIZE + 1) };
			int32_t col{ 1 };
			int32_t row{ 1 };

			context->AssetView(
				[&](auto* asset) {
					if (col++ <= row * colNo) {
						ImGui::SameLine();
					}
					else
						++row;

					//bool isClicked{};
					ImGui::ImageButtonEx(
						(ImGuiID)asset->uid,
						icon,
						ImVec2(ASSET_SIZE, ASSET_SIZE),
						ImVec2(0, 1), ImVec2(1, 0),
						ImVec4(0, 0, 0, 1),
						ImVec4(1, 1, 1, 1));
				}
			);
		}
		ImGui::End();
	}

private:
	Texture2D iconImage;
	ImTextureID icon;
	AssetID selected;
};