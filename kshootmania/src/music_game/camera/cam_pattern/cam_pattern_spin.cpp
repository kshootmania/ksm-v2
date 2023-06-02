#include "cam_pattern_spin.hpp"

namespace MusicGame::Camera
{
	CamPatternStatusSpin::CamPatternStatusSpin(kson::Pulse startPulse, const kson::CamPatternInvokeSpin& invoke)
		: m_startPulse(startPulse)
		, m_invoke(invoke)
	{
	}

	void CamPatternStatusSpin::apply(kson::Pulse currentPulse, CamStatus& camStatusRef) const
	{
		if (currentPulse < m_startPulse || m_startPulse + m_invoke.length <= currentPulse)
		{
			return;
		}

		const double rate = static_cast<double>(currentPulse - m_startPulse) / m_invoke.length;
		camStatusRef.rotationZLane += 360.0 * m_invoke.d * rate;
	}
}
