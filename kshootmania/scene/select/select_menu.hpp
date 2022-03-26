#pragma once
#include "select_menu_item.hpp"
#include "select_difficulty_menu.hpp"
#include "select_folder_state.hpp"
#include "ui/array_with_linear_menu.hpp"

class SelectMenu
{
private:
	SelectFolderState m_folderState;

	ArrayWithLinearMenu<SelectMenuItem> m_menu;

	SelectDifficultyMenu m_difficultyMenu;

	// TODO: Delete this
	Font m_debugFont;
	String m_debugStr;

	void decideSongItem();

	void decideDirectoryFolderItem();

	// Note: Set directoryPath to empty to close folder
	bool openDirectory(FilePathView directoryPath);

public:
	SelectMenu(); // TODO: Restore previous selection

	void update();

	void draw() const;

	void decide();

	bool isFolderOpen() const;

	void closeFolder();

	const SelectMenuItem& cursorMenuItem() const;

	const SelectMenuSongItemChartInfo* cursorChartInfoPtr() const;

	bool empty() const;
};
