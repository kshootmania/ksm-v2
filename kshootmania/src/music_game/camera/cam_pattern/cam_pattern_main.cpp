#include "cam_pattern_main.hpp"

namespace MusicGame::Camera
{
	CamPatternMain::CamPatternMain(const kson::ChartData& chartData)
		: m_spin(chartData)
		, m_halfSpin(chartData)
		, m_swing(chartData)
	{
	}

	void CamPatternMain::onLaserSlamJudged(kson::Pulse laserSlamPulse, int32 direction, kson::Pulse currentPulse)
	{
		m_spin.onLaserSlamJudged(laserSlamPulse, direction, currentPulse);
		m_halfSpin.onLaserSlamJudged(laserSlamPulse, direction, currentPulse);
		m_swing.onLaserSlamJudged(laserSlamPulse, direction, currentPulse);

	}

	void CamPatternMain::applyToCamStatus(CamStatus& camStatusRef, kson::Pulse currentPulse) const
	{
		m_spin.applyToCamStatus(camStatusRef, currentPulse);
		m_halfSpin.applyToCamStatus(camStatusRef, currentPulse);
		m_swing.applyToCamStatus(camStatusRef, currentPulse);
	}
}
