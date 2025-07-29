#include "cam_pattern_swing.hpp"

namespace MusicGame::Camera
{
	CamPatternSwing::CamPatternSwing(const kson::ChartData& chartData)
		: m_swingEvents(chartData.camera.cam.pattern.laser.slamEvent.swing)
		, m_alreadyInvokedEventPulses(m_swingEvents.size())
	{
	}

	void CamPatternSwing::onLaserSlamJudged(kson::Pulse laserSlamPulse, int32 direction, kson::Pulse currentPulse)
	{
		assert(direction == 1 || direction == -1);

		// 回転がなければ何もしない
		if (!m_swingEvents.contains(laserSlamPulse))
		{
			return;
		}

		// 既に再生済みの場合何もしない
		if (m_alreadyInvokedEventPulses.contains(laserSlamPulse))
		{
			return;
		}

		const auto& swingEvent = m_swingEvents.at(laserSlamPulse);

		// 直角LASERと回転方向が一致しない場合は何もしない
		if (swingEvent.d != direction)
		{
			return;
		}

		// 回転開始
		m_startPulse = currentPulse;
		m_durationRelPulse = swingEvent.length;
		m_direction = direction;
		m_alreadyInvokedEventPulses.insert(laserSlamPulse);
	}

	void CamPatternSwing::applyToCamStatus(CamStatus& camStatusRef, kson::Pulse currentPulse) const
	{
	}
}
