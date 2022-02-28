#include "ui/linear_menu.hpp"

void LinearMenu::update()
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
