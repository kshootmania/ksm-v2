#pragma once
#include "scoring/high_score_info.hpp"

struct ISelectMenuItemInfo
{
	virtual ~ISelectMenuItemInfo() = 0;
};
inline ISelectMenuItemInfo::~ISelectMenuItemInfo() = default;

struct SelectMenuItem
{
	enum ItemType
	{
		kUndefined = 0,

		kSong,
		kCourse,

		kAllFolder,
		kDirectoryFolder,
		kFavoriteFolder,

		kCurrentFolder,

		kSection,
	};

	ItemType itemType = kUndefined;

	FilePath fullPath;

	std::unique_ptr<ISelectMenuItemInfo> info = nullptr;
};

struct SelectMenuSongItemChartInfo
{
	String title;
	String artist;

	FilePath jacketFilePath;
	String jacketAuthor;

	FilePath chartFilePath;
	String chartAuthor;

	int32 level = 1;

	FilePath previewBGMFilePath;
	double previewBGMOffsetSec = 0.0;
	double previewBGMDurationSec = 15.0;

	FilePath iconFilePath;

	String information;

	HighScoreInfo highScoreInfo;
};

struct SelectMenuSongItemInfo : public ISelectMenuItemInfo
{
	using ChartInfos = std::array<Optional<SelectMenuSongItemChartInfo>, kNumDifficulties>;

	ChartInfos chartInfos;

	explicit SelectMenuSongItemInfo(const ChartInfos& chartInfos)
		: chartInfos(chartInfos)
	{
	}

	virtual ~SelectMenuSongItemInfo() = default;
};

struct SelectMenuFolderItemInfo : public ISelectMenuItemInfo
{
	String displayName;

	explicit SelectMenuFolderItemInfo(StringView displayName)
		: displayName(displayName)
	{
	}

	virtual ~SelectMenuFolderItemInfo() = default;
};

struct SelectMenuSectionItemInfo : public ISelectMenuItemInfo
{
	String displayName;

	explicit SelectMenuSectionItemInfo(StringView displayName)
		: displayName(displayName)
	{
	}

	virtual ~SelectMenuSectionItemInfo() = default;
};
