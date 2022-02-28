#pragma once
#include "ui/menu_helper.hpp"
#include "graphics/texture_atlas.hpp"

class OptionTopMenu
{
private:
	LinearMenu m_menu;

	TextureAtlas m_itemTextureAtlas;

	Stopwatch m_stopwatch;

public:
	enum Item : int32
	{
		kDisplaySound,
		kInputJudgment,
		kOther,
		kKeyConfig,

		kItemEnumCount,
	};

	OptionTopMenu();

	void update();

	void draw() const;

	template <typename T>
	T cursor()
	{
		return m_menu.cursor<T>();
	}
};
