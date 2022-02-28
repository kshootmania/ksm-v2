#pragma once
#include "ui/menu_helper.hpp"
#include "graphics/texture_atlas.hpp"

class TitleScene;

class TitleMenu
{
private:
	TitleScene* const m_pTitleScene;

	LinearMenu m_menu;

	TextureAtlas m_menuItemTextureAtlas;

	Texture m_menuCursorTexture;

	Stopwatch m_stopwatch;

	double m_easedCursorPos = 0.0;

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
