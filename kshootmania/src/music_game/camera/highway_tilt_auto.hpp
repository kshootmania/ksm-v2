#pragma once
#include "kson/chart_data.hpp"

namespace MusicGame::Camera
{
	class HighwayTiltAuto
	{
	private:
		double m_smoothedTiltFactor = 0.0;
		double m_prevTargetTiltFactor = 0.0;
		double m_tiltScale = 1.0;

		void updateTiltFactor(const kson::LaserLane<kson::LaserSection>& lanes, const kson::TiltInfo& tilt, kson::Pulse currentPulse);

		void updateTiltScale(const kson::LaserLane<kson::LaserSection>& lanes, const kson::TiltInfo& tilt, kson::Pulse currentPulse);

	public:
		HighwayTiltAuto() = default;

		void update(const kson::LaserLane<kson::LaserSection>& lanes, const kson::TiltInfo& tilt, kson::Pulse currentPulse);

		double radians() const;
	};
}
