#pragma once
#include "HighwayTiltAuto.hpp"
#include "HighwayTiltManual.hpp"
#include "kson/ChartData.hpp"

namespace MusicGame::Camera
{
	class HighwayTilt
	{
	private:
		HighwayTiltAuto m_auto;
		HighwayTiltManual m_manual;

	public:
		HighwayTilt() = default;

		void update(const kson::ChartData& chartData, kson::Pulse currentPulse);

		double radians() const;

		double radiansForBgLayer() const;
	};
}
