#pragma once
#include "Context/Context.h"

struct ResourceWindow : IWidget {
	BOOM_INLINE ResourceWindow(AppInterface* context) 
		: IWidget{ context }
		, iconImage{"Icons/asset.png", false}
		, icon{ (ImTextureID)iconImage }
		, selected{}
	{}

	BOOM_INLINE void OnShow(AppInterface* context) override {
		if (ImGui::Begin("Resources")) {
			int32_t colNo{ (int32_t)((ImGui::GetContentRegionAvail().x) / (ASSET_SIZE + ImGui::GetStyle().ItemSpacing.x)) };
			colNo = glm::max(1, colNo);

			ImGuiTableFlags flags{ 
				ImGuiTableFlags_SizingFixedSame | 
				ImGuiTableFlags_NoHostExtendX
			};

			if (ImGui::BeginTable("", colNo, flags)) {
				// set column sizes according to paddings
				for (int i{}; i < colNo; ++i) {
					ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, ASSET_SIZE);
				}

				context->AssetView(
					[&](auto* asset) {
						ImGui::TableNextColumn();

						bool isClicked{
							ImGui::ImageButtonEx(
								(ImGuiID)asset->uid,
								icon,
								ImVec2(ASSET_SIZE, ASSET_SIZE),
								ImVec2(0, 1), ImVec2(1, 0),
								ImVec4(0, 0, 0, 1),
								ImVec4(1, 1, 1, 1)) 
						};
						ImGui::TextWrapped(asset->source.c_str());

						//do stuff after click
						if (isClicked) {

						}
					}
				);
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

private:
	Texture2D iconImage;
	ImTextureID icon;
	AssetID selected;
};