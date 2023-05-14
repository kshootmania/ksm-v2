#include "highway_tilt_auto.hpp"
#include "music_game/game_defines.hpp"

namespace MusicGame::Camera
{
	namespace
	{
		constexpr double kSlowDownDiffThreshold = 0.1;
		constexpr double kZeroTiltFactorThreshold = 0.001;
		constexpr double kZeroTiltSlowDownFactor = 1.0 / 5;
		constexpr double kMinSpeed = 0.5;

		double Speed(double targetTiltFactor, double currentTiltFactor)
		{
			double speed = 4.5;

			// 目標の傾きに近くなった場合は減速させる
			const double diff = Abs(targetTiltFactor - currentTiltFactor);
			if (diff < kSlowDownDiffThreshold)
			{
				speed *= diff / kSlowDownDiffThreshold;
			}

			// 目標の傾きがゼロに近い場合は遅くする
			if (Abs(targetTiltFactor) < kZeroTiltFactorThreshold)
			{
				speed *= kZeroTiltSlowDownFactor;
			}

			// スピードが遅くなりすぎないようにする(目標値に収束させるため)
			speed = Max(speed, kMinSpeed);

			return speed;
		}
	}

	void HighwayTiltAuto::update(double tiltFactor)
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

	double HighwayTiltAuto::radians() const
	{
		return kTiltRadians * m_smoothedTiltFactor;
	}
}
