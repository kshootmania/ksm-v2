#include "HighwayTilt.hpp"

namespace MusicGame::Camera
{
	void HighwayTilt::update(const kson::ChartData& chartData, kson::Pulse currentPulse)
	{
		m_auto.update(chartData.note.laser, chartData.camera.tilt, currentPulse);
		m_manual.update(chartData.camera.tilt, currentPulse);
	}

	double HighwayTilt::radians() const
	{
		return std::lerp(m_auto.radians(), m_manual.radians(), m_manual.lerpRate());
	}

	double HighwayTilt::radiansForBgLayer() const
	{
		return std::lerp(m_auto.radiansForBgLayer(), m_manual.radiansForBgLayer(), m_manual.lerpRate());
	}
}
