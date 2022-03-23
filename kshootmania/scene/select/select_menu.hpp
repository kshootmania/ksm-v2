#pragma once
#include "select_menu_item.hpp"
#include "select_folder_state.hpp"

class SelectMenu
{
private:
	std::unique_ptr<LinearMenu> m_menu;
	Array<SelectMenuItem> m_items;

	SelectFolderState m_folderState;

	bool openDirectory(FilePathView directoryPath);

public:
	SelectMenu();

	bool isFolderOpen() const;

	void closeFolder();
};
