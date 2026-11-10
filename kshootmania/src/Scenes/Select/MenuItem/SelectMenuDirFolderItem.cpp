#include "SelectMenuDirFolderItem.hpp"
#include "Graphics/FontUtils.hpp"
#include "Common/FsUtils.hpp"

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

void SelectMenuDirFolderItem::setCanvasParamsCenter([[maybe_unused]] const SelectMenuEventContext& context, noco::Canvas& canvas, [[maybe_unused]] int32 difficultyIdx) const
{
	canvas.setSubCanvasParamValuesByTag(U"center", {
		{ U"isSong", false },
		{ U"isDirectory", true },
		{ U"isSubDirectory", false },
		{ U"isCourse", false },
		{ U"title", FolderDisplayNameCenter(m_displayName, m_isCurrentFolder) },
	});
}

void SelectMenuDirFolderItem::setCanvasParamsTopBottom([[maybe_unused]] const SelectMenuEventContext& context, noco::Canvas& canvas, [[maybe_unused]] int32 difficultyIdx, StringView tag) const
{
	canvas.setSubCanvasParamValuesByTag(tag, {
		{ U"isSong", false },
		{ U"isDirectory", true },
		{ U"isSubDirectory", false },
		{ U"isCourse", false },
		{ U"title", FolderDisplayNameTopBottom(m_displayName, m_isCurrentFolder) },
	});
}

void SelectMenuDirFolderItem::showInFileManager([[maybe_unused]] int32 difficultyIdx) const
{
	// フォルダをエクスプローラで開く
	System::ShowInFileManager(m_fullPath);
}

void SelectMenuDirFolderItem::requestGlyphPreload([[maybe_unused]] int32 difficultyIdx) const
{
	GlyphPreloadAddon::Request(AssetManagement::SystemFont(), m_displayName);
}
