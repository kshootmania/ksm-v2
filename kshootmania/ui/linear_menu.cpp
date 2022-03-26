#include "ui/linear_menu.hpp"

void LinearMenu::increment()
{
	m_cursor += m_cursorStep;

	const int32 range = m_cursorMax + 1 - m_cursorMin;
	if (m_cyclic && range > 0)
	{
		while (m_cursor > m_cursorMax)
		{
			m_cursor -= range;
		}
	}
	else
	{
		m_cursor = Min(m_cursor, m_cursorMax);
	}
}

void LinearMenu::decrement()
{
	m_cursor -= m_cursorStep;

	const int32 range = m_cursorMax + 1 - m_cursorMin;
	if (m_cyclic && range > 0)
	{
		while (m_cursor < m_cursorMin)
		{
			m_cursor += range;
		}
	}
	else
	{
		m_cursor = Max(m_cursor, m_cursorMin);
	}
}

void LinearMenu::update()
{
	const bool decrementKeyDown = KeyConfig::AnyButtonDown(m_decrementButtons);
	const bool incrementKeyDown = KeyConfig::AnyButtonDown(m_incrementButtons);

	if (m_pressedTimeStopwatch.has_value()) // Menu accepting hold-down
	{
		bool moveCursor = false;

		if (decrementKeyDown || incrementKeyDown)
		{
			m_pressedTimeStopwatch->restart();
			moveCursor = true;
		}

		const bool decrementKeyPressed = KeyConfig::AnyButtonPressed(m_decrementButtons);
		const bool incrementKeyPressed = KeyConfig::AnyButtonPressed(m_incrementButtons);
		const bool onlyDecrementKeyPressed = decrementKeyPressed && !incrementKeyPressed;
		const bool onlyIncrementKeyPressed = incrementKeyPressed && !decrementKeyPressed;

		const double pressedTimeSec = m_pressedTimeStopwatch->sF();
		if (!moveCursor && (onlyDecrementKeyPressed || onlyIncrementKeyPressed))
		{
			const int32 tickCount = std::max(static_cast<int>((pressedTimeSec - m_intervalSecFirst + m_intervalSec) / m_intervalSec), 0);
			const int32 tickCountPrev = std::max(static_cast<int>((m_pressedTimeSecPrev - m_intervalSecFirst + m_intervalSec) / m_intervalSec), 0);
			if (tickCount > tickCountPrev)
			{
				moveCursor = true;
			}
		}

		if (moveCursor)
		{
			if (onlyDecrementKeyPressed)
			{
				decrement();
			}
			else if (onlyIncrementKeyPressed)
			{
				increment();
			}
		}

		m_pressedTimeSecPrev = pressedTimeSec;
	}
	else // Menu not accepting hold-down
	{
		if (decrementKeyDown && !incrementKeyDown)
		{
			decrement();
		}
		else if (incrementKeyDown && !decrementKeyDown)
		{
			increment();
		}
	}
}

bool LinearMenu::isCursorMin() const
{
	return m_cursor <= m_cursorMin;
}

bool LinearMenu::isCursorMax() const
{
	return m_cursor >= m_cursorMax;
}
