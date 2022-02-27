#pragma once
#include "ui/menu_helper.hpp"
#include "graphics/texture_atlas.hpp"

class OptionScene;

class OptionMenu
{
public:
	enum Item : int32
	{
		kDisplaySound,
		kInputJudgment,
		kOther,
		kKeyConfig,

		kItemEnumCount,
	};

private:
	OptionScene* m_pOptionScene;

	LinearMenu<Item> m_menu;

	TextureAtlas m_menuItemTextureAtlas;

	Stopwatch m_stopwatch;

public:
	explicit OptionMenu(OptionScene* pOptionScene);

	void update();

	void draw() const;
};
