#pragma once
#include "graphics/tiled_texture.hpp"

class OptionTopMenu
{
private:
	LinearMenu m_menu;

	TiledTexture m_itemTiledTexture;

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
