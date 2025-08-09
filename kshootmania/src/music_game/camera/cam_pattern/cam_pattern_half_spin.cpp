#include "cam_pattern_half_spin.hpp"

namespace MusicGame::Camera
{
	CamPatternHalfSpin::CamPatternHalfSpin(const kson::ChartData& chartData)
		: m_halfSpinEvents(chartData.camera.cam.pattern.laser.slamEvent.halfSpin)
		, m_alreadyInvokedEventPulses(m_halfSpinEvents.size())
	{
	}

	void CamPatternHalfSpin::onLaserSlamJudged(kson::Pulse laserSlamPulse, int32 direction, kson::Pulse currentPulse)
	{
		assert(direction == 1 || direction == -1);

		// 半回転がなければ何もしない
		if (!m_halfSpinEvents.contains(laserSlamPulse))
		{
			return;
		}

		// 既に再生済みの場合何もしない
		if (m_alreadyInvokedEventPulses.contains(laserSlamPulse))
		{
			return;
		}

		const auto& halfSpinEvent = m_halfSpinEvents.at(laserSlamPulse);

		// 直角LASERと回転の方向が一致しない場合何もしない
		if (halfSpinEvent.d != direction)
		{
			return;
		}

		// 回転開始
		m_startPulse = currentPulse;
		m_durationRelPulse = halfSpinEvent.length;
		m_direction = direction;
		m_alreadyInvokedEventPulses.insert(laserSlamPulse);
	}

	void CamPatternHalfSpin::applyToCamStatus(CamStatus& camStatusRef, kson::Pulse currentPulse) const
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
			// 60度, 15度ずつ左右に揺らす
			// HSP版: https://github.com/kshootmania/ksm-v1/blob/b26026420fa164310bf25f93c218bb83480faef8/src/scene/play/play_game_logic.hsp#L233-L244
			double absDegrees;
			constexpr double kDuration = 400.0;
			if (rate < 110.0 / kDuration)
			{
				absDegrees = Sin(rate / (110.0 / kDuration) * 1.8) / Sin(1.8) * 60;
			}
			else if (rate < 220.0 / kDuration)
			{
				absDegrees = (Cos((rate * kDuration - 110.0) * 1.8 / 110.0) - Cos(1.8)) / (1.0 - Cos(1.8)) * 60;
			}
			else if (rate < 310.0 / kDuration)
			{
				absDegrees = -Sin((rate * kDuration - 220) * 1.8 / 90.0) / Sin(1.8) * 15;
			}
			else
			{
				absDegrees = -(Cos((rate * kDuration - 310) * 1.8 / 90.0) - Cos(1.8)) / (1.0 - Cos(1.8)) * 15;
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

		// 背景アニメーションの角度計算
		if (rate < 0.75)
		{
			camStatusRef.rotationZLayer += -m_direction * Sin(rate * 1.1 / 0.75) / Sin(1.1) * 360;
		}
	}
}
