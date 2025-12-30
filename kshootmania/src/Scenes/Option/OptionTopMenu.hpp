#pragma once
#include "UI/LinearMenu.hpp"

class OptionTopMenu
{
private:
	LinearMenu m_menu;

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

	template <typename T>
	T cursorAs()
	{
		return m_menu.cursorAs<T>();
	}

	void updateUI(noco::Canvas* pCanvas) const;
};
