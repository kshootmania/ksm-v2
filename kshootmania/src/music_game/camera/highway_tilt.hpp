#pragma once
#include "highway_tilt_auto.hpp"
#include "highway_tilt_manual.hpp"
#include "kson/chart_data.hpp"

namespace MusicGame::Camera
{
	class HighwayTilt
	{
	private:
		HighwayTiltAuto m_auto;
		HighwayTiltManual m_manual;

	public:
		HighwayTilt() = default;

		void update(double tiltFactor, const kson::ChartData& chartData, kson::Pulse currentPulse);

		double radians() const;
	};
}
