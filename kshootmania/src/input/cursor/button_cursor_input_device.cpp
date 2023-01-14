#include "button_cursor_input_device.hpp"

ButtonCursorInputDevice::ButtonCursorInputDevice(const Array<KeyConfig::Button>& incrementButtons, const Array<KeyConfig::Button>& decrementButtons, double intervalSec, double intervalSecFirst, StartRequiredForBTFXLaserYN startRequiredForBTFXLaser)
	: m_incrementButtons(incrementButtons)
	, m_decrementButtons(decrementButtons)
	, m_intervalSec(intervalSec)
	, m_intervalSecFirst(intervalSecFirst == 0.0 ? intervalSec : intervalSecFirst)
	, m_pressedTimeStopwatch(intervalSec == 0.0 ? none : MakeOptional<Stopwatch>(StartImmediately::No))
	, m_startRequiredForBTFXLaser(startRequiredForBTFXLaser)
{
}

ButtonCursorInputDevice::~ButtonCursorInputDevice() = default;

void ButtonCursorInputDevice::update()
{
	m_deltaCursor = 0;

	const bool decrementKeyDown = KeyConfig::AnyButtonDown(m_decrementButtons, m_startRequiredForBTFXLaser);
	const bool incrementKeyDown = KeyConfig::AnyButtonDown(m_incrementButtons, m_startRequiredForBTFXLaser);

	if (m_pressedTimeStopwatch.has_value()) // 押し続けてカーソル移動可能な場合
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

		const bool decrementKeyPressed = KeyConfig::AnyButtonPressed(m_decrementButtons, m_startRequiredForBTFXLaser);
		const bool incrementKeyPressed = KeyConfig::AnyButtonPressed(m_incrementButtons, m_startRequiredForBTFXLaser);
		const bool onlyDecrementKeyPressed = decrementKeyPressed && !incrementKeyPressed;
		const bool onlyIncrementKeyPressed = incrementKeyPressed && !decrementKeyPressed;

		const double pressedTimeSec = m_pressedTimeStopwatch->sF();
		if (!moveCursor && (onlyDecrementKeyPressed || onlyIncrementKeyPressed))
		{
			const int32 tickCount = (std::max)(static_cast<int32>((pressedTimeSec - m_intervalSecFirst + m_intervalSec) / m_intervalSec), 0);
			const int32 tickCountPrev = (std::max)(static_cast<int32>((m_pressedTimeSecPrev - m_intervalSecFirst + m_intervalSec) / m_intervalSec), 0);
			if (tickCount > tickCountPrev)
			{
				moveCursor = true;
			}
		}

		if (moveCursor)
		{
			if (decrementKeyDown || onlyDecrementKeyPressed)
			{
				--m_deltaCursor;
			}
			else if (incrementKeyDown || onlyIncrementKeyPressed)
			{
				++m_deltaCursor;
			}
		}

		m_pressedTimeSecPrev = pressedTimeSec;
	}
	else // 押し続けでのカーソル移動を受け付けない場合
	{
		if (decrementKeyDown && !incrementKeyDown)
		{
			--m_deltaCursor;
		}
		else if (incrementKeyDown && !decrementKeyDown)
		{
			++m_deltaCursor;
		}
	}
}

int32 ButtonCursorInputDevice::deltaCursor() const
{
	return m_deltaCursor;
}
