#pragma once
#include "ISelectMenuItem.hpp"

/// @brief 検索モード時に先頭へ表示する元フォルダへの戻り項目
class SelectMenuBackToFolderItem : public ISelectMenuItem
{
public:
	SelectMenuBackToFolderItem() = default;

	virtual ~SelectMenuBackToFolderItem() = default;

	virtual void decide(const SelectMenuEventContext& context, int32 difficultyIdx) override;

	virtual bool isFolder() const override
	{
		return true;
	}

	virtual FilePathView fullPath() const override
	{
		return FilePathView{};
	}

	virtual void setCanvasParamsCenter(const SelectMenuEventContext& context, noco::Canvas& canvas, int32 difficultyIdx) const override;

	virtual void setCanvasParamsTopBottom(const SelectMenuEventContext& context, noco::Canvas& canvas, int32 difficultyIdx, StringView tag) const override;
};
