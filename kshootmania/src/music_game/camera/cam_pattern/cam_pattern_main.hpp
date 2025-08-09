#pragma once
#include "kson/chart_data.hpp"
#include "music_game/camera/cam_system.hpp"
#include "cam_pattern_spin.hpp"
#include "cam_pattern_half_spin.hpp"
#include "cam_pattern_swing.hpp"

namespace MusicGame::Camera
{
	class CamPatternMain
	{
	private:
		CamPatternSpin m_spin;			// 一回転
		CamPatternHalfSpin m_halfSpin;	// 半回転
		CamPatternSwing m_swing;		// 左右振動

	public:
		explicit CamPatternMain(const kson::ChartData& chartData);

		void onLaserSlamJudged(kson::Pulse laserSlamPulse, int32 direction, kson::Pulse currentPulse);

		void applyToCamStatus(CamStatus& camStatusRef, kson::Pulse currentPulse) const;
	};
}
