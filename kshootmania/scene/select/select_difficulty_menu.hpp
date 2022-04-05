#pragma once
#include "ui/linear_menu.hpp"

class SelectMenu;

class SelectDifficultyMenu
{
private:
	LinearMenu m_menu;

	bool m_isCursorChanged = false;

	const SelectMenu* const m_pSelectMenu;

	const TiledTexture m_difficultyTexture;

	const TiledTexture m_levelNumberTexture;

	const Texture m_cursorTexture;

public:
	explicit SelectDifficultyMenu(const SelectMenu* pSelectMenu);

	void update();

	void draw() const;

	int32 cursor() const;

	int32 rawCursor() const;

	bool isCursorChanged() const;

	static int32 GetAlternativeCursor(int32 rawCursor, std::function<bool(int32)> difficultyExistsFunc);
};
