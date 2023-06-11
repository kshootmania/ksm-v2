#include "highway_tilt.hpp"

namespace MusicGame::Camera
{
	void HighwayTilt::update(const kson::ChartData& chartData, kson::Pulse currentPulse)
	{
		m_auto.update(chartData.note.laser, currentPulse);
		m_manual.update(chartData.camera.tilt.manual, currentPulse);
	}

	double HighwayTilt::radians() const
	{
		return std::lerp(m_auto.radians(), m_manual.radians(), m_manual.lerpRate());
	}
}
