#pragma once
#include "iselect_menu_item.hpp"

class SelectMenuDirFolderItem : public ISelectMenuItem
{
private:
	IsCurrentFolderYN m_isCurrentFolder;
	FilePath m_fullPath;
	String m_displayName;

public:
	explicit SelectMenuDirFolderItem(IsCurrentFolderYN isCurrentFolder, FilePathView fullPath);

	virtual ~SelectMenuDirFolderItem() = default;

	virtual void decide(const SelectMenuEventContext& context, int32 difficultyIdx) override;

	virtual FilePathView fullPath() const override
	{
		return m_fullPath;
	}

	virtual void drawCenter(int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets) const override;

	virtual void drawUpperLower(int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets, bool isUpper) const override;
};
