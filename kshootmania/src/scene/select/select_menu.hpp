#pragma once
#include "select_folder_state.hpp"
#include "ui/array_with_linear_menu.hpp"
#include "select_difficulty_menu.hpp"
#include "select_menu_graphics.hpp"
#include "select_song_preview.hpp"
#include "ksmaudio/ksmaudio.hpp"

using PlaySeYN = YesNo<struct PlaySeYN_tag>;

class ISelectMenuItem;

struct SelectMenuEventContext
{
	// Note: FilePathViewやconst FilePath&ではなくFilePathにしているのは意図的
	//       (メニュー項目の再構築が発生すると呼び出し元のFilePathが無効になるので、事前にコピーしておく必要がある)
	std::function<void(FilePath, MusicGame::IsAutoPlayYN)> fnMoveToPlayScene;
	std::function<void(FilePath)> fnOpenDirectory;
	std::function<void()> fnCloseFolder;
};

class SelectMenu
{
private:
	const SelectMenuEventContext m_eventContext;

	SelectFolderState m_folderState;

	ArrayWithLinearMenu<std::unique_ptr<ISelectMenuItem>> m_menu;

	SelectDifficultyMenu m_difficultyMenu;

	SelectMenuGraphics m_graphics;

	SelectMenuShakeDirection m_shakeDirection = SelectMenuShakeDirection::kUnspecified;
	Stopwatch m_shakeStopwatch;

	SelectSongPreview m_songPreview;

	const ksmaudio::Sample m_songSelectSe{"se/sel_m.wav"};

	const ksmaudio::Sample m_difficultySelectSe{"se/sel_l.wav"};

	const ksmaudio::Sample m_folderSelectSe{"se/sel_dir.wav"};

	bool openDirectory(FilePathView directoryPath, PlaySeYN playSe);

	void setCursorAndSave(int32 cursor);

	void setCursorToItemByFullPath(FilePathView fullPath);

	void refreshGraphics(SelectMenuGraphics::RefreshType type);

	void refreshSongPreview();

public:
	explicit SelectMenu(std::function<void(FilePathView, MusicGame::IsAutoPlayYN)> fnMoveToPlayScene);

	~SelectMenu(); // ヘッダではISelectMenuItemが不完全型なのでソースファイル側で定義

	void update();

	void draw() const;

	void decide();

	void decideAutoPlay();

	bool isFolderOpen() const;

	void closeFolder(PlaySeYN playSe);

	const ISelectMenuItem& cursorMenuItem() const;

	bool empty() const;

	void fadeOutSongPreviewForExit(double durationSec);
};
