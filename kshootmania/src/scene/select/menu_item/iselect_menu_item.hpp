#pragma once
#include "scene/select/select_menu.hpp"
#include "scene/select/select_chart_info.hpp"

struct SelectMenuItemGraphicAssets;

using IsCurrentFolderYN = YesNo<struct IsCurrentFolderYN_tag>;

class ISelectMenuItem
{
protected:
	const char m_slash = '/';

	static String FolderDisplayNameCenter(StringView folderName, IsCurrentFolderYN isCurrentFolder)
	{
		if (isCurrentFolder)
		{
			return U"<<   {}   <<"_fmt(folderName);
		}
		else
		{
			return U">>   {}   >>"_fmt(folderName);
		}
	}

	static String FolderDisplayNameUpperLower(StringView folderName, IsCurrentFolderYN isCurrentFolder)
	{
		if (isCurrentFolder)
		{
			return U"<<   {}     "_fmt(folderName);
		}
		else
		{
			return U"     {}   >>"_fmt(folderName);
		}
	}

public:
	ISelectMenuItem() = default;

	virtual ~ISelectMenuItem() = default;

	virtual void decide(const SelectMenuEventContext& context, int32 difficultyIdx) = 0;

	virtual void decideAutoPlay([[maybe_unused]] const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
	{
	}

	virtual FilePathView fullPath() const = 0;

	virtual const SelectChartInfo* chartInfoPtr([[maybe_unused]] int difficultyIdx) const
	{
		return nullptr;
	}

	virtual bool difficultyMenuExists() const
	{
		return false;
	}

	virtual void drawCenter(int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets) const = 0;

	virtual void drawUpperLower(int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets, bool isUpper) const = 0;
};
