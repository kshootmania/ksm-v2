#include "LinearMenu.hpp"
#include "Input/KeyConfig.hpp"

void LinearMenu::increment(int32 absDeltaCursor) noexcept
{
	const int32 range = m_cursorMax + 1 - m_cursorMin;
	if (m_cyclic && range > 0)
	{
		m_cursor += absDeltaCursor;
		while (m_cursor > m_cursorMax)
		{
			m_cursor -= range;
		}
	}
	else
	{
		if (m_cursor >= m_cursorMax)
		{
			m_cursor = m_cursorMax;
			m_deltaCursor = 0;
		}
		else
		{
			m_cursor += absDeltaCursor;
		}
	}
}

void LinearMenu::decrement(int32 absDeltaCursor) noexcept
{
	const int32 range = m_cursorMax + 1 - m_cursorMin;
	if (m_cyclic && range > 0)
	{
		m_cursor -= absDeltaCursor;
		while (m_cursor < m_cursorMin)
		{
			m_cursor += range;
		}
	}
	else
	{
		if (m_cursor <= m_cursorMin)
		{
			m_cursor = m_cursorMin;
			m_deltaCursor = 0;
		}
		else
		{
			m_cursor -= absDeltaCursor;
		}
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

[[nodiscard]]
int32 LinearMenu::cursor() const noexcept
{
	return m_cursor;
}

void LinearMenu::update() noexcept
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

void LinearMenu::setCursorStep(int32 step) noexcept
{
	m_cursorStep = step;
}

[[nodiscard]]
bool LinearMenu::isCursorMin() const noexcept
{
	return m_cursor <= m_cursorMin;
}

[[nodiscard]]
bool LinearMenu::isCursorMax() const noexcept
{
	return m_cursor >= m_cursorMax;
}

[[nodiscard]]
int32 LinearMenu::deltaCursor() const noexcept
{
	return m_deltaCursor;
}

[[nodiscard]]
double LinearMenu::cursorRate() const noexcept
{
	if (m_cursorMax <= m_cursorMin)
	{
		return 0.0;
	}
	return static_cast<double>(m_cursor - m_cursorMin) / static_cast<double>(m_cursorMax - m_cursorMin);
}
