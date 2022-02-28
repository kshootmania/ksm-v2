#pragma once
#include "input/key_config.hpp"

using IsCyclicMenu = YesNo<struct IsCyclicMenu_tag>;

class LinearMenu
{
private:
	const Array<KeyConfig::Button> m_incrementButtons;
	const Array<KeyConfig::Button> m_decrementButtons;

	const int32 m_cursorMin;
	const int32 m_cursorMax;

	int32 m_cursor;

	IsCyclicMenu m_cyclic;

	Optional<Timer> m_intervalTimer;

public:
	template <typename T>
	LinearMenu(
		T enumCount,
		const Array<KeyConfig::Button>& incrementButtons,
		const Array<KeyConfig::Button>& decrementButtons,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0);

	template <typename T>
	LinearMenu(
		T cursorMin,
		T cursorMax,
		const Array<KeyConfig::Button>& incrementButtons,
		const Array<KeyConfig::Button>& decrementButtons,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0);

	template <typename T = int32>
	T cursor() const;

	template <typename T>
	void setCursor(T value);

	void update();
};

template<typename T>
LinearMenu::LinearMenu(
	T enumCount,
	const Array<KeyConfig::Button>& incrementButtons,
	const Array<KeyConfig::Button>& decrementButtons,
	IsCyclicMenu cyclic,
	double intervalSec)
	: LinearMenu(
		static_cast<T>(0),
		static_cast<T>(static_cast<int32>(enumCount) - 1),
		incrementButtons,
		decrementButtons,
		cyclic,
		intervalSec)
{
}

template<typename T>
LinearMenu::LinearMenu(
	T cursorMin,
	T cursorMax,
	const Array<KeyConfig::Button>& incrementButtons,
	const Array<KeyConfig::Button>& decrementButtons,
	IsCyclicMenu cyclic,
	double intervalSec)
	: m_incrementButtons(incrementButtons)
	, m_decrementButtons(decrementButtons)
	, m_cursorMin(cursorMin)
	, m_cursorMax(cursorMax)
	, m_cursor(cursorMin)
	, m_cyclic(cyclic)
	, m_intervalTimer((intervalSec > 0.0) ? MakeOptional<Timer>(SecondsF(intervalSec), StartImmediately::No) : Optional<Timer>(none))
{
}

template<typename T>
T LinearMenu::cursor() const
{
	return static_cast<T>(m_cursor);
}

template<typename T>
void LinearMenu::setCursor(T value)
{
	m_cursor = Clamp(static_cast<int32>(value), m_cursorMin, m_cursorMax);
}
