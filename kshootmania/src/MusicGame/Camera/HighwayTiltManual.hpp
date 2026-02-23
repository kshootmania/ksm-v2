#pragma once
#include "kson/ChartData.hpp"

namespace MusicGame::Camera
{
	class HighwayTiltManual
	{
	private:
		double m_radians = 0.0;
		double m_lerpRate = 0.0;

	public:
		HighwayTiltManual() = default;

		void update(const kson::ByPulse<kson::TiltValue>& tilt, kson::Pulse currentPulse);

		double radians() const;

		double radiansForBgLayer() const;

		double lerpRate() const;
	};
}
