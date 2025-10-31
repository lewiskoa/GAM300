#pragma once
#include "Context/Context.h"

struct ResourceWindow : IWidget {
	BOOM_INLINE ResourceWindow(AppInterface* c) 
		: IWidget{ c }
		, iconImage{"Icons/asset.png", false}
		, icon{ (ImTextureID)iconImage }
		, selected{}
	{}

	BOOM_INLINE void OnShow() override {
		if (ImGui::Begin("Resources")) {
			
			if (ImGui::Button("Save All Assets", { 128, 20 })) {
				SaveAssets();
			}
			ImGui::SameLine();
			static bool isPopup{};
			if (ImGui::Button("Create Empty Material", { 140, 20 })) {
				isPopup = true;
			}
			if (isPopup) {
				ImGui::OpenPopup("Input Material Name");
				isPopup = CreateEmptyMaterial();
			}

			static int currentType{ static_cast<int>(AssetType::UNKNOWN) }; //unknown will show all assets
			ImGui::Combo("Filter", &currentType, TYPE_NAMES, IM_ARRAYSIZE(TYPE_NAMES));

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
					[&](Asset* asset) {
						if (static_cast<AssetType>(currentType) == AssetType::UNKNOWN || asset->type == static_cast<AssetType>(currentType)) {
							ImGui::TableNextColumn();
							//render with textures if avaliable
							ImTextureID texid{ icon }; //default file icon

							TextureAsset* tex{ dynamic_cast<TextureAsset*>(asset) };
							if (tex) texid = *tex->data.get();

							ImGui::PushID((int)asset->uid);
							bool isClicked = ImGui::ImageButton("##thumb", texid, ImVec2(ASSET_SIZE, ASSET_SIZE),
								ImVec2(0, 1), ImVec2(1, 0),
								ImVec4(0, 0, 0, 1),
								ImVec4(1, 1, 1, 1));

							if (tex && ImGui::BeginDragDropSource()) {
								ImGui::SetDragDropPayload(CONSTANTS::DND_PAYLOAD_TEXTURE.data(), &asset->uid, sizeof(AssetID));
								ImGui::Text("Dragging Texture: %s", asset->name.c_str());
								ImGui::EndDragDropSource();
							}
							else if (dynamic_cast<MaterialAsset*>(asset) && ImGui::BeginDragDropSource()) {
								ImGui::SetDragDropPayload(CONSTANTS::DND_PAYLOAD_MATERIAL.data(), &asset->uid, sizeof(AssetID));
								ImGui::Text("Dragging Material: %s", asset->name.c_str());
								ImGui::EndDragDropSource();
							}
							else if (dynamic_cast<ModelAsset*>(asset) && ImGui::BeginDragDropSource()) {
								ImGui::SetDragDropPayload(CONSTANTS::DND_PAYLOAD_MODEL.data(), &asset->uid, sizeof(AssetID));
								ImGui::Text("Dragging Model: %s", asset->name.c_str());
								ImGui::EndDragDropSource();
							}
							ImGui::PopID();

							ImGui::TextWrapped(asset->source.c_str());

							//show modifyable properties in inspector when selected
							if (isClicked) {
								context->SelectedAsset(true) = { asset->uid, asset->type };
							}
						}
					}
				);
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

private:
	BOOM_INLINE bool SaveAssets(const std::string& scenePath = "Scenes/")
	{
		//Try blocks cause crashed in release mode. Need to find new alternative
		DataSerializer serializer;

		const std::string assetsFilePath = scenePath + "assets.yaml";

		BOOM_INFO("[Assets] Saving assets to '{}'", assetsFilePath);

		// Serialize scene and assets
		serializer.Serialize(context->GetAssetRegistry(), assetsFilePath);

		BOOM_INFO("[Assets] Successfully saved assets");
		return true;
	}

	BOOM_INLINE bool CreateEmptyMaterial() {
		static char buff[CONSTANTS::CHAR_BUFFER_SIZE] = "New Material";
		
		if (ImGui::BeginPopupModal("Material Name", nullptr)) {
			ImGui::InputText("Name", buff, sizeof(buff));
			ImGui::Separator();
			
			bool shouldClose{};

			if (ImGui::Button("Close")) { //cancel operation
				ImGui::CloseCurrentPopup();
				shouldClose = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("OK")) { //create material operation
				ImGui::CloseCurrentPopup();
				std::string name{ buff };
				HandleConflictName(name);
				context->GetAssetRegistry().AddMaterial(RandomU64(), name);
				shouldClose = true;
			}

			ImGui::EndPopup();
			return shouldClose;
		}

		return true;
	}

	// Handle conflict: append a number to filename (e.g., "file (1).txt")
	BOOM_INLINE void HandleConflictName(std::string& name) {
		int counter{ 1 };
		bool duplicateName{true};
		std::string baseName{name};
		while (duplicateName) {
			duplicateName = false;
			context->AssetTypeView<MaterialAsset>([&baseName, &name, &counter, &duplicateName](MaterialAsset* mat) {
				if (mat->name == name) {
					baseName = name + " (" + std::to_string(counter) + ")";
					++counter;
					duplicateName = true;
					return;
				}
			});
		}
		name = baseName;
	}
private:
	Texture2D iconImage;
	ImTextureID icon;
	AssetID selected;

	bool showNamePopup{};
};