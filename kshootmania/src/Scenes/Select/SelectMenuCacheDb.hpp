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

struct SelectCachedChart
{
	FilePath chartFilePath;
	FilePath songDirectoryPath;
	int32 difficultyIdx = -1;
	std::shared_ptr<const SelectChartInfo> chartInfo;
};

class SelectMenuCacheDb
{
public:
	[[nodiscard]]
	static Array<SelectCachedSong> LoadDirectory(FilePathView directoryPath, bool forceRebuild);

	[[nodiscard]]
	static Array<SelectCachedSong> LoadDirectoryForNameSort(FilePathView directoryPath, bool forceRebuild);

	[[nodiscard]]
	static Array<SelectCachedSong> LoadDirectoryForAllNameSort(FilePathView directoryPath, bool forceRebuild);

	[[nodiscard]]
	static Optional<SelectCachedSong> FindSong(FilePathView songOrChartPath, bool forceRebuild);

	[[nodiscard]]
	static Optional<SelectCachedChart> FindChart(FilePathView chartPath, bool forceRebuild);

	static void Invalidate(FilePathView directoryPath);

	static void InvalidateAll();
};
