#include "cam_pattern_spin.hpp"

namespace MusicGame::Camera
{
	CamPatternSpin::CamPatternSpin(const kson::ChartData& chartData)
		: m_spinEvents(chartData.camera.cam.pattern.laser.slamEvent.spin)
        , m_alreadyInvokedEventPulses(m_spinEvents.size())
	{
	}

	void CamPatternSpin::onLaserSlamJudged(kson::Pulse laserSlamPulse, int32 direction, kson::Pulse currentPulse)
	{
		assert(direction == 1 || direction == -1);

		// 回転がなければ何もしない
		if (!m_spinEvents.contains(laserSlamPulse))
		{
			return;
		}

        // 既に再生済みの場合は何もしない
        if (m_alreadyInvokedEventPulses.contains(laserSlamPulse))
		{
			return;
		}

		const auto& spinEvent = m_spinEvents.at(laserSlamPulse);

		// 直角LASERと回転の方向が一致しない場合は何もしない
		if (spinEvent.d != direction)
		{
			return;
		}

		// 回転開始
		m_startPulse = currentPulse;
		m_durationRelPulse = spinEvent.length;
		m_direction = direction;
        m_alreadyInvokedEventPulses.insert(laserSlamPulse);
	}

	void CamPatternSpin::applyToCamStatus(CamStatus& camStatusRef, kson::Pulse currentPulse) const
    {
		if (currentPulse < m_startPulse)
		{
			return;
		}

		const kson::RelPulse elapsedPulse = currentPulse - m_startPulse;

		// ハイウェイの角度計算
		const double rate = static_cast<double>(elapsedPulse) / static_cast<double>(m_durationRelPulse);
		if (rate < 1.0)
		{
            // 30度だけ多めに回転させて揺り戻す
            // HSP版: https://github.com/kshootmania/ksm-v1/blob/b26026420fa164310bf25f93c218bb83480faef8/src/scene/play/play_game_logic.hsp#L205-L218
            double absDegrees;
            if (rate < 360.0 / 675)
            {
                absDegrees = Sin(rate / (360.0 / 675) * 0.75) / Sin(0.75) * 360;
            }
            else if (rate < 440.0 / 675)
            {
                absDegrees = Sin(Math::ToRadians((rate * 675 - 360.0) * 9 / 8)) * 30;
            }
            else
            {
                absDegrees = (1.0 - Pow(Cos(Math::ToRadians((1.0 - rate) * 90 / 235 * 675)), 2)) * 30;
            }

            const double degrees = -m_direction * absDegrees;
            camStatusRef.rotationZHighway += degrees;
		}

		// 判定ラインの角度計算
		if (rate < 1.5)
		{
			const double decayRate = (rate > 1.0) ? ((1.5 - rate) / (1.5 - 1.0)) : 1.0;
			camStatusRef.rotationZJdgline += -m_direction * Sin(Math::TwoPi * rate) * 20 * decayRate;
		}
    }
}
