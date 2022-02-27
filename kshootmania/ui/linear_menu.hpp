#pragma once
#include "input/key_config.hpp"

using IsCyclicMenu = YesNo<struct IsCyclicMenu_tag>;

template <typename T>
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
	LinearMenu(
		T enumCount,
		const Array<KeyConfig::Button>& incrementButtons,
		const Array<KeyConfig::Button>& decrementButtons,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0);

	LinearMenu(
		T cursorMin,
		T cursorMax,
		const Array<KeyConfig::Button>& incrementButtons,
		const Array<KeyConfig::Button>& decrementButtons,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0);

	T cursor() const;

	void setCursor(T value);

	void update();
};

template<typename T>
LinearMenu<T>::LinearMenu(
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
LinearMenu<T>::LinearMenu(
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
T LinearMenu<T>::cursor() const
{
	return static_cast<T>(m_cursor);
}

template<typename T>
void LinearMenu<T>::setCursor(T value)
{
	m_cursor = Clamp(static_cast<int32>(value), m_cursorMin, m_cursorMax);
}

template<typename T>
void LinearMenu<T>::update()
{
	if (m_intervalTimer.has_value()) // Menu accepting hold-down
	{
		if (m_intervalTimer->isStarted() && !m_intervalTimer->reachedZero())
		{
			return;
		}

		const bool decrementKeyPressed = KeyConfig::AnyButtonPressed(m_decrementButtons);
		const bool incrementKeyPressed = KeyConfig::AnyButtonPressed(m_incrementButtons);
		if (decrementKeyPressed && !incrementKeyPressed)
		{
			if (m_cursor > m_cursorMin)
			{
				--m_cursor;
				m_intervalTimer->restart();
			}
			else if (m_cyclic)
			{
				m_cursor = m_cursorMax;
				m_intervalTimer->restart();
			}
		}
		else if (incrementKeyPressed && !decrementKeyPressed)
		{
			if (m_cursor < m_cursorMax)
			{
				++m_cursor;
				m_intervalTimer->restart();
			}
			else if (m_cyclic)
			{
				m_cursor = 0;
				m_intervalTimer->restart();
			}
		}
	}
	else // Menu not accepting hold-down
	{
		const bool decrementKeyDown = KeyConfig::AnyButtonDown(m_decrementButtons);
		const bool incrementKeyDown = KeyConfig::AnyButtonDown(m_incrementButtons);
		if (decrementKeyDown && !incrementKeyDown)
		{
			if (m_cursor > m_cursorMin)
			{
				--m_cursor;
			}
			else if (m_cyclic)
			{
				m_cursor = m_cursorMax;
			}
		}
		else if (incrementKeyDown && !decrementKeyDown)
		{
			if (m_cursor < m_cursorMax)
			{
				++m_cursor;
			}
			else if (m_cyclic)
			{
				m_cursor = 0;
			}
		}
	}
}
