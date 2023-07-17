﻿#include "select_menu_dir_folder_item.hpp"
#include "scene/select/select_menu_graphics.hpp"

SelectMenuDirFolderItem::SelectMenuDirFolderItem(IsCurrentFolderYN isCurrentFolder, FilePathView fullPath)
	: m_isCurrentFolder(isCurrentFolder)
	, m_fullPath(fullPath)
	, m_displayName(FileSystem::FileName(m_fullPath))
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

	const ScopedRenderTarget2D scopedRenderTarget(renderTexture);
	assets.fontLL(FolderDisplayNameCenter(m_displayName, m_isCurrentFolder)).drawAt(Vec2{ 16 + 740 / 2, 135 + 102 / 2 });
}

void SelectMenuDirFolderItem::drawUpperLower([[maybe_unused]] int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets, bool isUpper) const
{
	Shader::Copy(isUpper ? assets.dirItemTextures.upperHalf : assets.dirItemTextures.lowerHalf, renderTexture);

	const ScopedRenderTarget2D scopedRenderTarget(renderTexture);
	assets.fontL(FolderDisplayNameUpperLower(m_displayName, m_isCurrentFolder)).drawAt(isUpper ? Vec2{ 16 + 770 / 2, 12 + 86 / 2 } : Vec2{ 16 + 770 / 2, 126 + 86 / 2 });
}
