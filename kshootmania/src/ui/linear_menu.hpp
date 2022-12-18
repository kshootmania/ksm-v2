#pragma once
#include <cassert>
#include "input/key_config.hpp"

using IsCyclicMenu = YesNo<struct IsCyclicMenu_tag>;

class LinearMenu
{
private:
	const Array<KeyConfig::Button> m_incrementButtons;
	const Array<KeyConfig::Button> m_decrementButtons;

	int32 m_cursorMin;
	int32 m_cursorMax;
	const int32 m_cursorStep;

	int32 m_cursor;

	IsCyclicMenu m_cyclic;

	Optional<Stopwatch> m_pressedTimeStopwatch;
	double m_pressedTimeSecPrev = 0.0;
	double m_intervalSec;
	double m_intervalSecFirst;

	bool m_isCursorIncremented = false;
	bool m_isCursorDecremented = false;

	void increment();

	void decrement();

public:
	template <typename T>
	LinearMenu(
		T enumCount,
		const Array<KeyConfig::Button>& incrementButtons,
		const Array<KeyConfig::Button>& decrementButtons,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0,
		double intervalSecFirst = 0.0,
		int32 defaultCursor = 0,
		int32 cursorStep = 1);

	template <typename T>
	LinearMenu(
		T cursorMin,
		T cursorMax,
		const Array<KeyConfig::Button>& incrementButtons,
		const Array<KeyConfig::Button>& decrementButtons,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0,
		double intervalSecFirst = 0.0,
		int32 defaultCursor = std::numeric_limits<int32>::min(),
		int32 cursorStep = 1);

	template <typename T = int32>
	T cursor() const;

	template <typename T>
	void setCursor(T value);

	void update();

	template <typename T>
	void setCursorMin(T value);

	template <typename T>
	void setCursorMax(T value);

	bool isCursorMin() const;

	bool isCursorMax() const;

	bool isCursorChanged() const;

	bool isCursorIncremented() const;

	bool isCursorDecremented() const;
};

template<typename T>
LinearMenu::LinearMenu(
		T enumCount,
		const Array<KeyConfig::Button>& incrementButtons,
		const Array<KeyConfig::Button>& decrementButtons,
		IsCyclicMenu cyclic,
		double intervalSec,
		double intervalSecFirst,
		int32 defaultCursor,
		int32 cursorStep)
	: LinearMenu(
		static_cast<T>(0),
		static_cast<T>(static_cast<int32>(enumCount) - 1),
		incrementButtons,
		decrementButtons,
		cyclic,
		intervalSec,
		intervalSecFirst,
		defaultCursor,
		cursorStep)
{
}

template<typename T>
LinearMenu::LinearMenu(
		T cursorMin,
		T cursorMax,
		const Array<KeyConfig::Button>& incrementButtons,
		const Array<KeyConfig::Button>& decrementButtons,
		IsCyclicMenu cyclic,
		double intervalSec,
		double intervalSecFirst,
		int32 defaultCursor,
		int32 cursorStep)
	: m_incrementButtons(incrementButtons)
	, m_decrementButtons(decrementButtons)
	, m_cursorMin(static_cast<int32>(cursorMin))
	, m_cursorMax(static_cast<int32>(cursorMax))
	, m_cursorStep(cursorStep)
	, m_cursor(Clamp(defaultCursor, static_cast<int32>(cursorMin), static_cast<int32>(cursorMax)))
	, m_cyclic(cyclic)
	, m_pressedTimeStopwatch(intervalSec == 0.0 ? none : MakeOptional<Stopwatch>(StartImmediately::No))
	, m_intervalSec(intervalSec)
	, m_intervalSecFirst(intervalSecFirst == 0.0 ? intervalSec : intervalSecFirst)
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
	int32 cursor = static_cast<int32>(value);

	if (m_cyclic && m_cursorMin < m_cursorMax)
	{
		const int32 range = m_cursorMax + 1 - m_cursorMin;
		if (cursor < m_cursorMin)
		{
			while (cursor < m_cursorMin)
			{
				cursor += range;
			}
		}
		else
		{
			while (cursor > m_cursorMax)
			{
				cursor -= range;
			}
		}
	}

	m_cursor = Clamp(cursor, m_cursorMin, m_cursorMax);
}

template<typename T>
void LinearMenu::setCursorMin(T value)
{
	assert(value <= m_cursorMax);

	m_cursorMin = value;
	if (value > m_cursorMax)
	{
		Print << U"LinearMenu::setCursorMax(): value must be smaller than or equal to m_cursorMax";
		m_cursorMin = value;
	}
	m_cursor = Clamp(m_cursor, m_cursorMin, m_cursorMax);
}

template<typename T>
void LinearMenu::setCursorMax(T value)
{
	assert(value >= m_cursorMin);

	m_cursorMax = value;
	if (value < m_cursorMin)
	{
		Print << U"LinearMenu::setCursorMax(): value must be larger than or equal to m_cursorMin";
		m_cursorMax = value;
	}
	m_cursor = Clamp(m_cursor, m_cursorMin, m_cursorMax);
}
