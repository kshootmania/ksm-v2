#include "SelectMenuBackToFolderItem.hpp"

void SelectMenuBackToFolderItem::decide(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
{
	if (context.fnExitSearch)
	{
		context.fnExitSearch();
	}
}

void SelectMenuBackToFolderItem::setCanvasParamsCenter([[maybe_unused]] const SelectMenuEventContext& context, noco::Canvas& canvas, [[maybe_unused]] int32 difficultyIdx) const
{
	canvas.setSubCanvasParamValuesByTag(U"center", {
		{ U"isSong", false },
		{ U"isDirectory", true },
		{ U"isSubDirectory", false },
		{ U"isCourse", false },
		{ U"title", FolderDisplayNameCenter(I18n::Get(I18n::Select::SearchCloseResult), IsCurrentFolderYN::Yes) },
	});
}

void SelectMenuBackToFolderItem::setCanvasParamsTopBottom([[maybe_unused]] const SelectMenuEventContext& context, noco::Canvas& canvas, [[maybe_unused]] int32 difficultyIdx, StringView tag) const
{
	canvas.setSubCanvasParamValuesByTag(tag, {
		{ U"isSong", false },
		{ U"isDirectory", true },
		{ U"isSubDirectory", false },
		{ U"isCourse", false },
		{ U"title", FolderDisplayNameTopBottom(I18n::Get(I18n::Select::SearchCloseResult), IsCurrentFolderYN::Yes) },
	});
}
