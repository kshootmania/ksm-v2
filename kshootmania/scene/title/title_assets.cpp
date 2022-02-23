#include "title_assets.hpp"

void TitleAssets::RegisterAssets()
{
	TextureAsset::Register(TitleTexture::kBG, U"imgs/standby.jpg");
	TextureAsset::Register(TitleTexture::kMenuItem, U"imgs/standby_item.gif");
	TextureAsset::Register(TitleTexture::kMenuCursor, U"imgs/standby_cur.gif");
}
