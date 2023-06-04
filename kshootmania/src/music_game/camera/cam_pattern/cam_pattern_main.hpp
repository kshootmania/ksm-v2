#pragma once
#include "kson/chart_data.hpp"
#include "music_game/camera/cam_system.hpp"
#include "cam_pattern_spin.hpp"

namespace MusicGame::Camera
{
	class CamPatternMain
	{
	private:
		CamPatternSpin m_spin;

	public:
		explicit CamPatternMain(const kson::ChartData& chartData);

		void onLaserSlamJudged(kson::Pulse laserSlamPulse, int32 direction, kson::Pulse currentPulse);

		void applyToCamStatus(CamStatus& camStatusRef, kson::Pulse currentPulse) const;
	};
}
