#pragma once
#include "iselect_menu_item.hpp"

class SelectMenuFavFolderItem : public ISelectMenuItem
{
private:
	const IsCurrentFolderYN m_isCurrentFolder;
	const FilePath m_fullPath;
	const String m_displayName;

public:
	explicit SelectMenuFavFolderItem(IsCurrentFolderYN isCurrentFolder, FilePathView fullPath);

	virtual ~SelectMenuFavFolderItem() = default;

	virtual void decide(const SelectMenuEventContext& context, int32 difficultyIdx) override;

	virtual FilePathView fullPath() const override
	{
		return m_fullPath;
	}

	virtual void drawCenter(int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets) const override;

	virtual void drawUpperLower(int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets, bool isUpper) const override;
};
