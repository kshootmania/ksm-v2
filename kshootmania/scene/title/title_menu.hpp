#pragma once
#include "ui/menu_helper.hpp"
#include "graphics/texture_atlas.hpp"

class TitleScene;

class TitleMenu
{
public:
	enum Item : int32
	{
		kStart = 0,
		kOption,
		kInputGate,
		kExit,

		kItemEnumCount,
	};

private:
	TitleScene* m_pTitleScene;

	LinearMenu<Item> m_menu;

	TextureAtlas m_menuItemTextureAtlas;

	Texture m_menuCursorTexture;

	Stopwatch m_stopwatch;

	double m_easedCursorPos = 0.0;

public:
	explicit TitleMenu(TitleScene* pTitleScene);

	void update();

	void draw() const;
};
