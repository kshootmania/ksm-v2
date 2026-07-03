#include "ClickHoldRepeat.hpp"

ClickHoldRepeat::ClickHoldRepeat(double intervalSec, double intervalSecFirst)
	: m_intervalSec(intervalSec)
	, m_intervalSecFirst(intervalSecFirst == 0.0 ? intervalSec : intervalSecFirst)
	, m_pressedTimeStopwatch(StartImmediately::No)
{
}

int32 ClickHoldRepeat::update(bool pressed)
{
	if (!pressed)
	{
		m_pressedTimeStopwatch.reset();
		m_pressedTimeSecPrev = 0.0;
		return 0;
	}

	if (!m_pressedTimeStopwatch.isRunning())
	{
		// 押下開始フレームでは1回発火
		m_pressedTimeStopwatch.restart();
		m_pressedTimeSecPrev = 0.0;
		return 1;
	}

	const double pressedTimeSec = m_pressedTimeStopwatch.sF();
	const int32 tickCount = (std::max)(static_cast<int32>((pressedTimeSec - m_intervalSecFirst + m_intervalSec) / m_intervalSec), 0);
	const int32 tickCountPrev = (std::max)(static_cast<int32>((m_pressedTimeSecPrev - m_intervalSecFirst + m_intervalSec) / m_intervalSec), 0);
	m_pressedTimeSecPrev = pressedTimeSec;
	return tickCount - tickCountPrev;
}
