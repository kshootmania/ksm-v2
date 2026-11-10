#include "SelectMenuSubDirSectionItem.hpp"
#include "Common/FsUtils.hpp"

SelectMenuSubDirSectionItem::SelectMenuSubDirSectionItem(FilePathView fullPath, const Optional<String>& customDisplayName)
	: m_fullPath(fullPath)
	, m_displayName(customDisplayName.value_or(FsUtils::DirectoryNameByDirectoryPath(m_fullPath)))
{
}

void SelectMenuSubDirSectionItem::decide(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
{
	context.fnMoveToNextSubDirSection();
}

void SelectMenuSubDirSectionItem::decideAutoPlay(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
{
	context.fnMoveToPrevSubDirSection();
}

void SelectMenuSubDirSectionItem::setCanvasParamsCenter([[maybe_unused]] const SelectMenuEventContext& context, noco::Canvas& canvas, [[maybe_unused]] int32 difficultyIdx) const
{
	canvas.setSubCanvasParamValuesByTag(U"center", {
		{ U"isSong", false },
		{ U"isDirectory", false },
		{ U"isSubDirectory", true },
		{ U"isCourse", false },
		{ U"title", m_displayName },
	});
}

void SelectMenuSubDirSectionItem::setCanvasParamsTopBottom([[maybe_unused]] const SelectMenuEventContext& context, noco::Canvas& canvas, [[maybe_unused]] int32 difficultyIdx, StringView tag) const
{
	canvas.setSubCanvasParamValuesByTag(tag, {
		{ U"isSong", false },
		{ U"isDirectory", false },
		{ U"isSubDirectory", true },
		{ U"isCourse", false },
		{ U"title", m_displayName },
	});
}

void SelectMenuSubDirSectionItem::showInFileManager([[maybe_unused]] int32 difficultyIdx) const
{
	// サブフォルダをエクスプローラで開く
	System::ShowInFileManager(m_fullPath);
}

void SelectMenuSubDirSectionItem::requestGlyphPreload([[maybe_unused]] int32 difficultyIdx) const
{
	GlyphPreloadAddon::Request(AssetManagement::SystemFont(), m_displayName);
}
