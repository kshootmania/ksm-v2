#include "SelectMenuFavFolderItem.hpp"
#include "Common/FsUtils.hpp"

SelectMenuFavFolderItem::SelectMenuFavFolderItem(IsCurrentFolderYN isCurrentFolder, FilePathView specialPath)
	: m_isCurrentFolder(isCurrentFolder)
	, m_fullPath(specialPath)
	, m_displayName(specialPath.substr(1))
{
}

void SelectMenuFavFolderItem::decide(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
{
	if (m_isCurrentFolder)
	{
		context.fnCloseFolder();
	}
	else
	{
		context.fnOpenFavoriteFolder(m_fullPath);
	}
}

void SelectMenuFavFolderItem::setCanvasParamsCenter([[maybe_unused]] const SelectMenuEventContext& context, noco::Canvas& canvas, [[maybe_unused]] int32 difficultyIdx) const
{
	canvas.setSubCanvasParamValuesByTag(U"center", {
		{ U"isSong", false },
		{ U"isDirectory", true },
		{ U"isSubDirectory", false },
		{ U"isCourse", false },
		{ U"title", FolderDisplayNameCenter(m_displayName, m_isCurrentFolder) },
	});
}

void SelectMenuFavFolderItem::setCanvasParamsTopBottom([[maybe_unused]] const SelectMenuEventContext& context, noco::Canvas& canvas, [[maybe_unused]] int32 difficultyIdx, StringView tag) const
{
	canvas.setSubCanvasParamValuesByTag(tag, {
		{ U"isSong", false },
		{ U"isDirectory", true },
		{ U"isSubDirectory", false },
		{ U"isCourse", false },
		{ U"title", FolderDisplayNameTopBottom(m_displayName, m_isCurrentFolder) },
	});
}

void SelectMenuFavFolderItem::showInFileManager([[maybe_unused]] int32 difficultyIdx) const
{
	// .favファイルがあるディレクトリをエクスプローラで開く
	const FilePath favFilePath = FileSystem::PathAppend(FsUtils::SongsDirectoryPath(), m_displayName + U".fav");
	if (FileSystem::IsFile(favFilePath))
	{
		System::ShowInFileManager(favFilePath);
	}
}

void SelectMenuFavFolderItem::requestGlyphPreload([[maybe_unused]] int32 difficultyIdx) const
{
	GlyphPreloadAddon::Request(AssetManagement::SystemFont(), m_displayName);
}
