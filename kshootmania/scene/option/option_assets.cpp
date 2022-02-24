#include "option_assets.hpp"

void OptionAssets::RegisterAssets()
{
	TextureAsset::Register(OptionTexture::kBG, U"imgs/bg.jpg");
	TextureAsset::Register(OptionTexture::kTopMenuItem, U"imgs/config_item.gif");
}
