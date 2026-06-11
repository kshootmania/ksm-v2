#pragma once
#include "SelectFolderState.hpp"
#include "SelectHighScoreCache.hpp"
#include "UI/ArrayWithLinearMenu.hpp"
#include "SelectDifficultyMenu.hpp"
#include "SelectSongPreview.hpp"
#include "ksmaudio/ksmaudio.hpp"
#include "Course/CoursePlayState.hpp"

using PlaySeYN = YesNo<struct PlaySeYN_tag>;
using RefreshSongPreviewYN = YesNo<struct RefreshSongPreviewYN_tag>;
using SaveToConfigIniYN = YesNo<struct SaveToConfigIniYN_tag>;
using SongPreviewOnlyYN = YesNo<struct SongPreviewOnlyYN_tag>;
using ReloadFromDiskYN = YesNo<struct ReloadFromDiskYN_tag>;

enum class SelectMenuShakeDirection
{
	kUnspecified,
	kUp,
	kDown,
};

class ISelectMenuItem;

struct SelectMenuEventContext
{
	// Note: FilePathViewやconst FilePath&ではなくFilePathにしているのは意図的
	//       (メニュー項目の再構築が発生すると呼び出し元のFilePathが無効になるので、事前にコピーしておく必要がある)
	std::function<void(FilePath, MusicGame::IsAutoPlayYN, const Optional<CoursePlayState>&)> fnMoveToPlayScene;
	std::function<void(FilePath)> fnOpenDirectory;
	std::function<void()> fnOpenAllFolder;
	std::function<void(FilePath)> fnOpenFavoriteFolder;
	std::function<void()> fnOpenCoursesFolder;
	std::function<void()> fnCloseFolder;
	std::function<void()> fnExitSearch;
	std::function<void()> fnMoveToNextSubDirSection;
	std::function<void()> fnMoveToPrevSubDirSection;
	std::function<void(int32)> fnChangeDifficulty;
	std::function<void(int32, int32)> fnJumpToItemWithDifficulty;
	std::function<void(StringView)> fnShowErrorDialog;

	// 譜面のハイスコア情報を取得
	std::function<HighScoreInfo(FilePathView)> fnGetHighScore;

	// コースのハイスコア情報を取得
	std::function<HighScoreInfo(FilePathView)> fnGetCourseHighScore;
};

class SelectMenu
{
private:
	static constexpr int32 kNumDisplayItems = 8;
	static constexpr int32 kNumTopItems = kNumDisplayItems / 2;
	static constexpr int32 kNumBottomItems = kNumDisplayItems - kNumTopItems - 1;

	const SelectMenuEventContext m_eventContext;

	std::shared_ptr<noco::Canvas> m_selectSceneCanvas;

	SelectFolderState m_folderState;

	bool m_searchActive = false;

	String m_searchQuery;

	// 検索フィルタ有効時のカーソル保持先の譜面ファイルパス
	Optional<FilePath> m_searchPreservedChartPath;

	ArrayWithLinearMenu<std::unique_ptr<ISelectMenuItem>> m_menu;

	SelectDifficultyMenu m_difficultyMenu;

	SelectSongPreview m_songPreview;

	// 譜面・コースのハイスコア情報のキャッシュ
	SelectHighScoreCache m_highScoreCache;

	const ksmaudio::Sample m_songSelectSe{"se/sel_m.wav"};

	const ksmaudio::Sample m_difficultySelectSe{"se/sel_l.wav"};

	const ksmaudio::Sample m_folderSelectSe{"se/sel_dir.wav"};

	bool openDirectory(FilePathView directoryPath, PlaySeYN playSe, RefreshSongPreviewYN refreshSongPreview = RefreshSongPreviewYN::Yes, SaveToConfigIniYN saveToConfigIni = SaveToConfigIniYN::Yes);

	bool openDirectoryWithNameSort(FilePathView directoryPath);

	bool openDirectoryWithLevelSort(FilePathView directoryPath);

	bool openAllFolder(PlaySeYN playSe, RefreshSongPreviewYN refreshSongPreview = RefreshSongPreviewYN::Yes, SaveToConfigIniYN saveToConfigIni = SaveToConfigIniYN::Yes);

	bool openAllFolderWithNameSort();

	bool openAllFolderWithLevelSort();

	bool openFavoriteFolder(FilePathView specialPath, PlaySeYN playSe, RefreshSongPreviewYN refreshSongPreview = RefreshSongPreviewYN::Yes, SaveToConfigIniYN saveToConfigIni = SaveToConfigIniYN::Yes);

	bool openFavoriteFolderWithNameSort(FilePathView specialPath);

	bool openFavoriteFolderWithLevelSort(FilePathView specialPath);

	bool openCoursesFolder(PlaySeYN playSe, RefreshSongPreviewYN refreshSongPreview = RefreshSongPreviewYN::Yes, SaveToConfigIniYN saveToConfigIni = SaveToConfigIniYN::Yes);

	bool openCoursesFolderWithNameSort();

	bool openCoursesFolderWithLevelSort();

	void setCursorAndSave(int32 cursor);

	void setCursorToItemByFullPath(FilePathView fullPath);

	void resetSongInfoTextDisplayTimer();

	void refreshContentCanvasParams();

	void refreshSongPreview();

	void playShakeUpTween();

	void playShakeDownTween();

	Array<FilePath> getSortedFolderPaths() const;

	Optional<std::size_t> findFolderIndex(const Array<FilePath>& folderPaths, FilePathView targetFullPath) const;

	void addOtherFolderItemsRotated();

	void addOtherFolderItemsSimple();

	void applySearchModeFilter();

public:
	explicit SelectMenu(
		const std::shared_ptr<noco::Canvas>& selectSceneCanvas,
		std::function<void(FilePathView, MusicGame::IsAutoPlayYN, const Optional<CoursePlayState>&)> fnMoveToPlayScene,
		std::function<void(StringView)> fnShowErrorDialog,
		std::function<void()> fnExitSearchMode);

	~SelectMenu(); // ヘッダではISelectMenuItemが不完全型なのでソースファイル側で定義

	void update(SongPreviewOnlyYN songPreviewOnly = SongPreviewOnlyYN::No);

	void decide();

	void decideAutoPlay();

	[[nodiscard]]
	bool isFolderOpen() const;

	void closeFolder(PlaySeYN playSe);

	[[nodiscard]]
	const ISelectMenuItem& cursorMenuItem() const;

	[[nodiscard]]
	bool empty() const;

	void fadeOutSongPreviewForExit(Duration duration);

	void reloadCurrentDirectory(RefreshSongPreviewYN refreshSongPreview, ReloadFromDiskYN reloadFromDisk);

	void forceReloadCurrentDirectory();

	void clearHighScoreCache();

	// ハイスコア表示を更新
	void refreshHighScoreDisplay();

	void jumpToAlphabetItem(char32 letter);

	void setCursorByChartFilePath(FilePathView chartFilePath);

	[[nodiscard]]
	Optional<FilePath> currentChartFilePath() const;

	void moveToNextSubDirSection();

	void moveToPrevSubDirSection();

	void jumpToNextAlphabet();

	void jumpToPrevAlphabet();

	void jumpToFirst();

	void jumpToLast();

	[[nodiscard]]
	double getCurrentChartStdBPM() const;

	[[nodiscard]]
	Optional<HighScoreInfo> getCurrentHighScoreInfo() const;

	void showCurrentItemInFileManager();

	void openCurrentChartIRPage();

	[[nodiscard]]
	Optional<String> currentItemRelativePathToCopy() const;

	[[nodiscard]]
	const SelectFolderState& folderState() const;

	/// @brief 検索モードに入る
	/// @param preservedChartPath カーソルを保持する譜面ファイルパス
	void enterSearchMode(const Optional<FilePath>& preservedChartPath = none);

	/// @brief 検索モードを抜ける
	void exitSearchMode();

	/// @brief 維持対象譜面パスを更新
	/// @param chartPath カーソルを保持する譜面ファイルパス
	void setSearchPreservedChartPath(const Optional<FilePath>& chartPath);

	/// @brief 検索クエリを設定
	/// @param query 検索クエリ
	void setSearchQuery(StringView query);

	[[nodiscard]]
	bool isSearchActive() const;

	[[nodiscard]]
	const String& searchQuery() const;
};
