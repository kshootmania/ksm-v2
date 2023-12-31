#include "select_menu_fav_folder_item.hpp"
#include "scene/select/select_menu_graphics.hpp"

SelectMenuFavFolderItem::SelectMenuFavFolderItem(IsCurrentFolderYN isCurrentFolder, FilePathView fullPath)
	: m_isCurrentFolder(isCurrentFolder)
	, m_fullPath(fullPath)
	, m_displayName(FileSystem::FileName(m_fullPath))
{
}

void SelectMenuFavFolderItem::decide(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
{
	//Print << U"Not Implemented (SelectMenuFavFolderItem::decide)";
}

void SelectMenuFavFolderItem::drawCenter([[maybe_unused]] int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets) const
{
	Shader::Copy(assets.dirItemTextures.center, renderTexture);

	const ScopedRenderTarget2D scopedRenderTarget(renderTexture);
	assets.font(FolderDisplayNameCenter(m_displayName, m_isCurrentFolder)).drawAt(44, Vec2{ 16 + 740 / 2, 135 + 102 / 2 });
}

void SelectMenuFavFolderItem::drawUpperLower([[maybe_unused]] int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets, bool isUpper) const
{
	Shader::Copy(isUpper ? assets.dirItemTextures.upperHalf : assets.dirItemTextures.lowerHalf, renderTexture);

	const ScopedRenderTarget2D scopedRenderTarget(renderTexture);
	assets.font(FolderDisplayNameUpperLower(m_displayName, m_isCurrentFolder)).drawAt(38, isUpper ? Vec2{ 16 + 770 / 2, 12 + 86 / 2 } : Vec2{ 16 + 770 / 2, 126 + 86 / 2 });
}
