#pragma once
#include "iselect_menu_item.hpp"

class SelectMenuSongItem : public ISelectMenuItem
{
private:
	const FilePath m_fullPath;

	bool m_chartExists = false;

	std::array<std::unique_ptr<SelectChartInfo>, kNumDifficulties> m_chartInfos;

public:
	explicit SelectMenuSongItem(const FilePath& songDirectoryPath);

	virtual ~SelectMenuSongItem() = default;

	virtual void decide(const SelectMenuEventContext& context, int32 difficultyIdx) override;

	virtual void decideAutoPlay(const SelectMenuEventContext& context, int32 difficultyIdx) override;

	virtual FilePathView fullPath() const override
	{
		return m_fullPath;
	}

	virtual const SelectChartInfo* chartInfoPtr(int difficultyIdx) const override;

	virtual bool difficultyMenuExists() const override
	{
		return true;
	}

	virtual void drawCenter(int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets) const override;

	virtual void drawUpperLower(int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets, bool isUpperHalf) const override;

	bool chartExists() const
	{
		return m_chartExists;
	}
};
