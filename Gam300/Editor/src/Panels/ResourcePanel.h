#pragma once
#include "Context/Context.h"


class ResourcePanel : public IWidget
{
public:
    BOOM_INLINE ResourcePanel(AppInterface* c);
    BOOM_INLINE void OnShow() override;

private:
    Texture2D iconImage;
    ImTextureID icon;
    AssetID selected;

    static constexpr float ASSET_SIZE = 64.0f;
};
