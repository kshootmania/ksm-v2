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
	if (m_intervalTimer.has_value()) // Menu accepting hold-down
	{
		if (m_intervalTimer->isStarted() && !m_intervalTimer->reachedZero())
		{
			return;
		}

		const int32 cursorPrev = m_cursor;

		const bool decrementKeyPressed = KeyConfig::AnyButtonPressed(m_decrementButtons);
		const bool incrementKeyPressed = KeyConfig::AnyButtonPressed(m_incrementButtons);
		if (decrementKeyPressed && !incrementKeyPressed)
		{
			decrement();
		}
		else if (incrementKeyPressed && !decrementKeyPressed)
		{
			increment();
		}

		if (m_cursor != cursorPrev)
		{
			m_intervalTimer->restart();
		}
	}
	else // Menu not accepting hold-down
	{
		const bool decrementKeyDown = KeyConfig::AnyButtonDown(m_decrementButtons);
		const bool incrementKeyDown = KeyConfig::AnyButtonDown(m_incrementButtons);
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
