#pragma once
#include "UI/LinearMenu.hpp"

class OptionTopMenu
{
private:
	LinearMenu m_menu;

	bool m_clickDecided = false;

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

	void update(noco::Canvas* pCanvas);

	/// @brief カーソル位置の項目がクリックで決定されたかどうかを返す
	[[nodiscard]]
	bool clickDecided() const;

	template <typename T>
	T cursorAs()
	{
		return m_menu.cursorAs<T>();
	}

	void updateUI(noco::Canvas* pCanvas) const;
};
