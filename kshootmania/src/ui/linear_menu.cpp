#include "ui/linear_menu.hpp"

void LinearMenu::increment()
{
	const int32 cursorPrev = m_cursor;

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

	if (m_cursor != cursorPrev || (m_cyclic && m_cursorMin == m_cursorMax))
	{
		m_isCursorIncremented = true;
	}
}

void LinearMenu::decrement()
{
	const int32 cursorPrev = m_cursor;

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

	if (m_cursor != cursorPrev || (m_cyclic && m_cursorMin == m_cursorMax))
	{
		m_isCursorDecremented = true;
	}
}

void LinearMenu::update()
{
	m_isCursorIncremented = false;
	m_isCursorDecremented = false;

	const bool decrementKeyDown = KeyConfig::AnyButtonDown(m_decrementButtons);
	const bool incrementKeyDown = KeyConfig::AnyButtonDown(m_incrementButtons);

	if (m_pressedTimeStopwatch.has_value()) // Menu accepting hold-down
	{
		bool moveCursor = false;

		if (decrementKeyDown || incrementKeyDown)
		{
			if (!m_pressedTimeStopwatch->isStarted())
			{
				m_pressedTimeStopwatch->start();
			}
			else
			{
				m_pressedTimeStopwatch->restart();
			}
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

bool LinearMenu::isCursorChanged() const
{
	return m_isCursorIncremented || m_isCursorDecremented;
}

bool LinearMenu::isCursorIncremented() const
{
	return m_isCursorIncremented;
}

bool LinearMenu::isCursorDecremented() const
{
	return m_isCursorDecremented;
}
