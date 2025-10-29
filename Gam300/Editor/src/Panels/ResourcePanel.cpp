#include "Panels/ResourcePanel.h"
#include <glm/glm.hpp>

BOOM_INLINE ResourcePanel::ResourcePanel(AppInterface* c)
    : IWidget(c)
    , iconImage("Icons/asset.png", false)
    , icon((ImTextureID)iconImage)
    , selected{}
{
}

BOOM_INLINE void ResourcePanel::OnShow()
{
    if (ImGui::Begin("Resources"))
    {
        int colNo = static_cast<int>((ImGui::GetContentRegionAvail().x) / (ASSET_SIZE + ImGui::GetStyle().ItemSpacing.x));
        colNo = glm::max(1, colNo);

        ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_NoHostExtendX;

        if (ImGui::BeginTable("##ResourceTable", colNo, flags))
        {
            for (int i = 0; i < colNo; ++i)
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, ASSET_SIZE);

            context->AssetView([&](auto* asset)
                {
                    ImGui::TableNextColumn();

                    ImTextureID texid = icon;
                    if (auto tex = dynamic_cast<TextureAsset*>(asset))
                        texid = *tex->data.get();

                    bool clicked = ImGui::ImageButtonEx(
                        (ImGuiID)asset->uid,
                        texid,
                        ImVec2(ASSET_SIZE, ASSET_SIZE),
                        ImVec2(0, 1), ImVec2(1, 0),
                        ImVec4(0, 0, 0, 1),
                        ImVec4(1, 1, 1, 1));

                    ImGui::TextWrapped(asset->source.c_str());

                    if (clicked)
                    {
                        selected = asset->uid;
                        // future: preview / inspector integration
                    }
                });

            ImGui::EndTable();
        }
    }
    ImGui::End();
}
