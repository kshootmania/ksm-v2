#pragma once
#include "SelectChartInfo.hpp"

struct SelectCachedSong
{
	FilePath songDirectoryPath;
	FilePath sectionDirectoryPath;
	Optional<String> sectionDisplayName;
	bool skipSectionHeading = false;
	bool visibleInDirectoryNameSort = true;
	bool visibleInAllNameSort = true;
	int32 sectionDirectoryIndex = -1;
	int32 songDirectoryIndex = -1;
	String songSortKey;
	std::array<std::shared_ptr<const SelectChartInfo>, kNumDifficulties> chartInfos = {};
};

struct SelectCachedSongEntry
{
	SelectCachedSong song;
	bool isSingleChartItem = false;
};

/// @brief レベルソート用の譜面1件分のキャッシュエントリ
struct SelectCachedChartEntry
{
	std::shared_ptr<const SelectChartInfo> chartInfo;
	int32 difficultyIdx = 0;
	String songSortKey;
};

class SelectMenuCacheDb
{
public:
	[[nodiscard]]
	static Array<SelectCachedSongEntry> LoadDirectoryForNameSort(FilePathView directoryPath, bool forceRebuild);

	[[nodiscard]]
	static Array<SelectCachedSongEntry> LoadDirectoryForAllNameSort(FilePathView directoryPath, bool forceRebuild);

	[[nodiscard]]
	static Array<SelectCachedChartEntry> LoadDirectoryForLevelSort(FilePathView directoryPath, bool forceRebuild);

	static void Invalidate(FilePathView directoryPath);

	static void InvalidateAll();
};
