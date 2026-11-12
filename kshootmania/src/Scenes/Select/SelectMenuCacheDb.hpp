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
	/// @brief ディレクトリの譜面情報を複数フレームに分けて読み込み
	/// @remark 読み込み完了後、同じディレクトリのLoadDirectoryFor...はファイルの変更確認なしで呼ぶと読み込み済みの内容を返す
	class Loader
	{
	public:
		struct Progress
		{
			int32 loadedChartCount = 0;
			int32 totalChartCount = 0;
		};

	private:
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;

	public:
		Loader(const Array<FilePath>& directoryPaths, bool checkFileChanges);

		~Loader();

		Loader(Loader&&) noexcept;

		Loader& operator=(Loader&&) noexcept;

		/// @brief 読み込みを進める
		/// @param timeLimit 1回の呼び出しで読み込みに使う時間
		void advance(Duration timeLimit);

		[[nodiscard]]
		bool isFinished() const;

		/// @brief 読み込みの進捗を取得
		/// @return 進捗(全譜面数が未確定の場合はnone)
		[[nodiscard]]
		Optional<Progress> progress() const;
	};

	[[nodiscard]]
	static Array<SelectCachedSongEntry> LoadDirectoryForNameSort(FilePathView directoryPath, bool forceRebuild, bool checkFileChanges = true);

	[[nodiscard]]
	static Array<SelectCachedSongEntry> LoadDirectoryForAllNameSort(FilePathView directoryPath, bool forceRebuild, bool checkFileChanges = true);

	[[nodiscard]]
	static Array<SelectCachedChartEntry> LoadDirectoryForLevelSort(FilePathView directoryPath, bool forceRebuild, bool checkFileChanges = true);

	static void Invalidate(FilePathView directoryPath);

	static void InvalidateAll();
};
