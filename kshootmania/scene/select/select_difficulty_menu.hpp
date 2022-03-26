#pragma once
#include "ui/linear_menu.hpp"

class SelectMenu;

class SelectDifficultyMenu
{
private:
	LinearMenu m_menu;

	bool m_isCursorChanged = false;

	const SelectMenu* const m_pSelectMenu;

public:
	explicit SelectDifficultyMenu(const SelectMenu* pSelectMenu);

	void update();

	int32 cursor() const;

	int32 rawCursor() const;
};
