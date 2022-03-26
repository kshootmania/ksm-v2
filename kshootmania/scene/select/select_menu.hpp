#pragma once
#include "select_menu_item.hpp"
#include "select_folder_state.hpp"
#include "ui/array_with_linear_menu.hpp"

class SelectMenu
{
private:
	ArrayWithLinearMenu<SelectMenuItem> m_menu;

	SelectFolderState m_folderState;

	// Note: Set directoryPath to empty to close folder
	bool openDirectory(FilePathView directoryPath);

public:
	SelectMenu();

	void update();

	bool isFolderOpen() const;

	void closeFolder();
};
