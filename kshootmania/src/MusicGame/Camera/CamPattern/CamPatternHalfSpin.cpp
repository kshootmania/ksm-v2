#include "CamPatternHalfSpin.hpp"

namespace MusicGame::Camera
{
	CamPatternHalfSpin::CamPatternHalfSpin(const kson::ChartData& chartData)
		: m_halfSpinEvents(chartData.camera.cam.pattern.laser.slamEvent.halfSpin)
		, m_alreadyInvokedEventPulses(m_halfSpinEvents.size())
	{
	}

	void CamPatternHalfSpin::onLaserSlamJudged(kson::Pulse laserSlamPulse, int32 direction, kson::Pulse currentPulse)
	{
		assert(direction == 1 || direction == -1);

		if (!m_halfSpinEvents.contains(laserSlamPulse))
		{
			return;
		}

		if (m_alreadyInvokedEventPulses.contains(laserSlamPulse))
		{
			return;
		}

		const auto& halfSpinEvent = m_halfSpinEvents.at(laserSlamPulse);

		if (halfSpinEvent.d != direction)
		{
			return;
		}

		m_startPulse = currentPulse;
		m_durationRelPulse = halfSpinEvent.length;
		m_direction = direction;
		m_deg = halfSpinEvent.v.deg;
		m_alreadyInvokedEventPulses.insert(laserSlamPulse);
	}

	void CamPatternHalfSpin::applyToCamStatus(CamStatus& camStatusRef, kson::Pulse currentPulse) const
	{
		if (currentPulse < m_startPulse)
		{
			return;
		}

		const kson::RelPulse elapsedPulse = currentPulse - m_startPulse;

		const double rate = static_cast<double>(elapsedPulse) / static_cast<double>(m_durationRelPulse);
		if (rate < 1.0)
		{
			double absDegrees;
			constexpr double kDuration = 400.0;
			if (rate < 110.0 / kDuration)
			{
				absDegrees = Sin(rate / (110.0 / kDuration) * 1.8) / Sin(1.8) * m_deg;
			}
			else if (rate < 220.0 / kDuration)
			{
				absDegrees = (Cos((rate * kDuration - 110.0) * 1.8 / 110.0) - Cos(1.8)) / (1.0 - Cos(1.8)) * m_deg;
			}
			else if (rate < 310.0 / kDuration)
			{
				absDegrees = -Sin((rate * kDuration - 220) * 1.8 / 90.0) / Sin(1.8) * m_deg / 4;
			}
			else
			{
				absDegrees = -(Cos((rate * kDuration - 310) * 1.8 / 90.0) - Cos(1.8)) / (1.0 - Cos(1.8)) * m_deg / 4;
			}

			const double degrees = -m_direction * absDegrees;
			camStatusRef.rotationDegHighway += degrees;
		}

		if (rate < 1.5)
		{
			const double decayRate = (rate > 1.0) ? ((1.5 - rate) / (1.5 - 1.0)) : 1.0;
			camStatusRef.rotationDegJdgline += -m_direction * Sin(Math::TwoPi * rate) * 20 * decayRate;
		}

		if (rate < 0.75)
		{
			camStatusRef.spinDegForLayer += -m_direction * Sin(rate * 1.1 / 0.75) / Sin(1.1) * 360;
		}
	}
}
