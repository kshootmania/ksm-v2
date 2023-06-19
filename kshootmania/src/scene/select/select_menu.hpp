#pragma once
#include "select_folder_state.hpp"
#include "ui/array_with_linear_menu.hpp"
#include "select_menu_item.hpp"
#include "select_difficulty_menu.hpp"
#include "select_menu_graphics.hpp"
#include "ksmaudio/ksmaudio.hpp"

class SelectMenu
{
private:
	SelectFolderState m_folderState;

	ArrayWithLinearMenu<SelectMenuItem> m_menu;

	SelectDifficultyMenu m_difficultyMenu;

	SelectMenuGraphics m_graphics;

	SelectMenuShakeDirection m_shakeDirection = SelectMenuShakeDirection::kUnspecified;
	Stopwatch m_shakeStopwatch;

	const std::function<void(FilePathView)> m_moveToPlaySceneFunc;

	const ksmaudio::Sample m_songSelectSe{"se/sel_m.wav"};

	const ksmaudio::Sample m_difficultySelectSe{"se/sel_l.wav"};

	// TODO: Delete this
	Font m_debugFont;
	String m_debugStr;

	void decideSongItem();

	void decideDirectoryFolderItem();

	bool openDirectory(FilePathView directoryPath);

	void setCursorAndSave(int32 cursor);

	void setCursorToItemByFullPath(FilePathView fullPath);

	void refreshGraphics(SelectMenuGraphics::RefreshType type);

public:
	explicit SelectMenu(std::function<void(FilePathView)> moveToPlaySceneFunc); // TODO: Restore previous selection

	void update();

	void draw() const;

	void decide();

	bool isFolderOpen() const;

	void closeFolder();

	const SelectMenuItem& cursorMenuItem() const;

	const SelectMenuSongItemChartInfo* cursorChartInfoPtr() const;

	bool empty() const;
};
