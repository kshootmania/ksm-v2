#include "HighwayTiltManual.hpp"
#include "MusicGame/GameDefines.hpp"

namespace MusicGame::Camera
{
	void HighwayTiltManual::update(const kson::ByPulse<kson::TiltValue>& tilt, kson::Pulse currentPulse)
	{
		const std::optional<double> value = kson::ManualTiltValueAt(tilt, currentPulse);

		m_lerpRate = Clamp(m_lerpRate + Scene::DeltaTime() / 0.04 * (value.has_value() ? 1 : -1), 0.0, 1.0);
		if (value.has_value())
		{
			m_radians = kTiltRadians * value.value();
		}
	}

	double HighwayTiltManual::radians() const
	{
		return m_radians;
	}

	double HighwayTiltManual::radiansForBgLayer() const
	{
		const double deg = Math::ToDegrees(m_radians);
		const double wrapped = MathUtils::WrappedFmod(deg + 180.0, 360.0) - 180.0;
		const double clamped = Clamp(wrapped, -14.0, 14.0);
		const double absWrapped = Abs(wrapped);
		const double factor1 = 1.0 - Max(absWrapped, 90.0) / 180.0;
		const double factor2 = 1.0 - Abs(Max(absWrapped, 90.0) - 90.0) / 90.0;
		return Math::ToRadians(clamped * factor1 * factor2);
	}

	double HighwayTiltManual::lerpRate() const
	{
		return m_lerpRate;
	}
}
