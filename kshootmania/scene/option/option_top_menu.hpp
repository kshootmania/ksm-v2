#pragma once
#include "ui/menu_helper.hpp"
#include "graphics/texture_atlas.hpp"

class OptionTopMenu
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
	LinearMenu<Item> m_menu;

	TextureAtlas m_itemTextureAtlas;

	Stopwatch m_stopwatch;

public:
	OptionTopMenu();

	void update();

	void draw() const;
};
