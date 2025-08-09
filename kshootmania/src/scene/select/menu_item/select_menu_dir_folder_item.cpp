#include "select_menu_dir_folder_item.hpp"
#include "scene/select/select_menu_graphics.hpp"
#include "graphics/font_utils.hpp"

SelectMenuDirFolderItem::SelectMenuDirFolderItem(IsCurrentFolderYN isCurrentFolder, FilePathView fullPath)
	: m_isCurrentFolder(isCurrentFolder)
	, m_fullPath(fullPath)
	, m_displayName(FsUtils::DirectoryNameByDirectoryPath(m_fullPath))
{
}

void SelectMenuDirFolderItem::decide(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
{
	if (m_isCurrentFolder)
	{
		context.fnCloseFolder();
	}
	else
	{
		context.fnOpenDirectory(m_fullPath);
	}
}

void SelectMenuDirFolderItem::drawCenter([[maybe_unused]] int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets) const
{
	Shader::Copy(assets.dirItemTextures.center, renderTexture);

	const String displayName = FolderDisplayNameCenter(m_displayName, m_isCurrentFolder);

	const ScopedRenderTarget2D scopedRenderTarget(renderTexture);
	FontUtils::DrawTextCenterWithFitWidth(assets.fontBold(displayName), 44, 42, { 36, 135, 700, 102 });
}

void SelectMenuDirFolderItem::drawUpperLower([[maybe_unused]] int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets, bool isUpper) const
{
	Shader::Copy(isUpper ? assets.dirItemTextures.upperHalf : assets.dirItemTextures.lowerHalf, renderTexture);

	const String displayName = FolderDisplayNameUpperLower(m_displayName, m_isCurrentFolder);

	const ScopedRenderTarget2D scopedRenderTarget(renderTexture);
	FontUtils::DrawTextCenterWithFitWidth(assets.font(displayName), 38, 36, isUpper ? Rect{ 26, 17, 750, 86 } : Rect{ 26, 131, 750, 86 });
}
