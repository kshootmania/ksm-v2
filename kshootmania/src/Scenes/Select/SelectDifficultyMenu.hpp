#pragma once
#include "UI/LinearMenu.hpp"

class SelectMenu;

class SelectDifficultyMenu
{
private:
	LinearMenu m_menu;

	const SelectMenu* const m_pSelectMenu;

	Stopwatch m_stopwatch;

public:
	explicit SelectDifficultyMenu(const SelectMenu* pSelectMenu);

	void update(bool* pCursorChanged);

	int32 cursor() const;

	void setCursor(int32 cursor);

	int32 rawCursor() const;

	static int32 GetAlternativeCursor(int32 rawCursor, std::function<bool(int32)> fnDifficultyExists);
};
