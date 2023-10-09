#include "select_menu_all_folder_item.hpp"
#include "scene/select/select_menu_graphics.hpp"

namespace
{
	constexpr StringView kDisplayName = U"All";
}

SelectMenuAllFolderItem::SelectMenuAllFolderItem(IsCurrentFolderYN isCurrentFolder)
	: m_isCurrentFolder(isCurrentFolder)
{
}

void SelectMenuAllFolderItem::decide(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
{
	Print << U"Not Implemented (SelectMenuAllFolderItem::decide)";
}

void SelectMenuAllFolderItem::drawCenter([[maybe_unused]] int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets) const
{
	Shader::Copy(assets.dirItemTextures.center, renderTexture);

	const ScopedRenderTarget2D scopedRenderTarget(renderTexture);
	assets.font(FolderDisplayNameCenter(kDisplayName, m_isCurrentFolder)).drawAt(44, Vec2{ 16 + 740 / 2, 135 + 102 / 2 });
}

void SelectMenuAllFolderItem::drawUpperLower([[maybe_unused]] int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets, bool isUpper) const
{
	Shader::Copy(isUpper ? assets.dirItemTextures.upperHalf : assets.dirItemTextures.lowerHalf, renderTexture);

	const ScopedRenderTarget2D scopedRenderTarget(renderTexture);
	assets.font(FolderDisplayNameUpperLower(kDisplayName, m_isCurrentFolder)).drawAt(38, isUpper ? Vec2{ 16 + 770 / 2, 12 + 86 / 2 } : Vec2{ 16 + 770 / 2, 126 + 86 / 2 });
}
