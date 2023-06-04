#pragma once
#include "kson/chart_data.hpp"

namespace MusicGame::Camera
{
	struct CamStatus
	{
		double zoom = 0.0;
		double shiftX = 0.0;
		double rotationX = 0.0;
		double rotationZ = 0.0;
		double rotationZHighway = 0.0;
		double rotationZJdgline = 0.0;
		double centerSplit;
	};

	class CamSystem
	{
	private:
		CamStatus m_status;

	public:
		CamSystem() = default;

		void update(const kson::ChartData& chartData, kson::Pulse currentPulse);

		const CamStatus& status() const
		{
			return m_status;
		}
	};
}
