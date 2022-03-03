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
	const int32 m_cursorStep;

	int32 m_cursor;

	IsCyclicMenu m_cyclic;

	Optional<Timer> m_intervalTimer;

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
		int32 defaultCursor = std::numeric_limits<int32>::min(),
		int32 cursorStep = 1);

	template <typename T = int32>
	T cursor() const;

	template <typename T>
	void setCursor(T value);

	void update();

	bool isCursorMin() const;

	bool isCursorMax() const;
};

template<typename T>
LinearMenu::LinearMenu(
		T enumCount,
		const Array<KeyConfig::Button>& incrementButtons,
		const Array<KeyConfig::Button>& decrementButtons,
		IsCyclicMenu cyclic,
		double intervalSec,
		int32 defaultCursor,
		int32 cursorStep)
	: LinearMenu(
		static_cast<T>(0),
		static_cast<T>(static_cast<int32>(enumCount) - 1),
		incrementButtons,
		decrementButtons,
		cyclic,
		intervalSec,
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
		int32 defaultCursor,
		int32 cursorStep)
	: m_incrementButtons(incrementButtons)
	, m_decrementButtons(decrementButtons)
	, m_cursorMin(static_cast<int32>(cursorMin))
	, m_cursorMax(static_cast<int32>(cursorMax))
	, m_cursorStep(cursorStep)
	, m_cursor(Clamp(defaultCursor, static_cast<int32>(cursorMin), static_cast<int32>(cursorMax)))
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
