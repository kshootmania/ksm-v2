#pragma once
#include "iselect_menu_item.hpp"

class SelectMenuAllFolderItem : public ISelectMenuItem
{
private:
	IsCurrentFolderYN m_isCurrentFolder;

public:
	static constexpr StringView kAllFolderSpecialPath = U"*"; // FullPathがこの値の場合は例外的にAllフォルダを表す

	explicit SelectMenuAllFolderItem(IsCurrentFolderYN isCurrentFolder);

	virtual ~SelectMenuAllFolderItem() = default;

	virtual FilePathView fullPath() const override
	{
		return kAllFolderSpecialPath;
	}

	virtual void decide(const SelectMenuEventContext& context, int32 difficultyIdx) override;

	virtual void drawCenter(int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets) const override;

	virtual void drawUpperLower(int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets, bool isUpper) const override;
};
