#include "highway_tilt.hpp"

namespace
{
	constexpr double kSlowDownDiffThreshold = 0.1;
	constexpr double kZeroTiltFactorThreshold = 0.001;
	constexpr double kZeroTiltSlowDownFactor = 1.0 / 5;
	constexpr double kMinSpeed = 0.5;

	double Speed(double targetTiltFactor, double currentTiltFactor)
	{
		double speed = 4.5;

		// Slow down if close to target tilt factor
		const double diff = Abs(targetTiltFactor - currentTiltFactor);
		if (diff < kSlowDownDiffThreshold)
		{
			speed *= diff / kSlowDownDiffThreshold;
		}

		// Slow down if the target is close to zero
		if (Abs(targetTiltFactor) < kZeroTiltFactorThreshold)
		{
			speed *= kZeroTiltSlowDownFactor;
		}

		// Do not make the speed too small
		speed = Max(speed, kMinSpeed);

		return speed;
	}
}

void MusicGame::Graphics::HighwayTilt::update(double tiltFactor)
{
	const double speed = Speed(tiltFactor, m_smoothedTiltFactor);

	if (m_smoothedTiltFactor < tiltFactor)
	{
		m_smoothedTiltFactor = Min(m_smoothedTiltFactor + Scene::DeltaTime() * speed, tiltFactor);
	}
	else
	{
		m_smoothedTiltFactor = Max(m_smoothedTiltFactor - Scene::DeltaTime() * speed, tiltFactor);
	}
}

double MusicGame::Graphics::HighwayTilt::radians() const
{
	return -10_deg * m_smoothedTiltFactor;
}
