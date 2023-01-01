#include "linear_menu.hpp"

void LinearMenu::increment(int32 absDeltaCursor)
{
	m_cursor += absDeltaCursor;

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

void LinearMenu::decrement(int32 absDeltaCursor)
{
	m_cursor -= absDeltaCursor;

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

LinearMenu::LinearMenu(const CreateInfoWithEnumCount& createInfo)
	: m_cursorInput(createInfo.cursorInputCreateInfo)
	, m_cursorMin(0)
	, m_cursorMax(createInfo.enumCount - 1)
	, m_cursor(Clamp(createInfo.defaultCursor, 0, createInfo.enumCount - 1))
	, m_cursorStep(1)
	, m_cyclic(createInfo.cyclic)
{
}

LinearMenu::LinearMenu(const CreateInfoWithCursorMinMax& createInfo)
	: m_cursorInput(createInfo.cursorInputCreateInfo)
	, m_cursorMin(createInfo.cursorMin)
	, m_cursorMax(createInfo.cursorMax)
	, m_cursor(Clamp(createInfo.defaultCursor, createInfo.cursorMin, createInfo.cursorMax))
	, m_cursorStep(createInfo.cursorStep)
	, m_cyclic(createInfo.cyclic)
{
}

void LinearMenu::update()
{
	m_cursorInput.update();
	m_deltaCursor = m_cursorInput.deltaCursor();

	const int32 absDeltaCursor = Abs(m_deltaCursor) * m_cursorStep;
	if (m_deltaCursor > 0)
	{
		increment(absDeltaCursor);
	}
	else if (m_deltaCursor < 0)
	{
		decrement(absDeltaCursor);
	}
}

void LinearMenu::setCursorStep(int32 step)
{
	m_cursorStep = step;
}

bool LinearMenu::isCursorMin() const
{
	return m_cursor <= m_cursorMin;
}

bool LinearMenu::isCursorMax() const
{
	return m_cursor >= m_cursorMax;
}

int32 LinearMenu::deltaCursor() const
{
	return m_deltaCursor;
}
