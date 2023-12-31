#pragma once
#include "graphics/tiled_texture.hpp"
#include "ksmaudio/ksmaudio.hpp"

class TitleScene;

class TitleMenu
{
private:
	TitleScene* const m_pTitleScene;

	LinearMenu m_menu;

	TiledTexture m_menuItemTexture;

	TiledTexture m_menuCursorTexture;

	Stopwatch m_stopwatch;

	double m_easedCursorPos = 0.0;

	ksmaudio::Sample m_selectSe{ "se/title_sel.wav" };

public:
	enum Item : int32
	{
		kStart = 0,
		kOption,
		kInputGate,
		kExit,

		kItemEnumCount,
	};

	explicit TitleMenu(TitleScene* pTitleScene);

	void update();

	void draw() const;
};
