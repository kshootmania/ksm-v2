#pragma once
#include "kson/chart_data.hpp"
#include "music_game/game_defines.hpp"
#include "music_game/camera/cam_system.hpp"

namespace MusicGame::Camera
{
	class CamPatternSwing
	{
	private:
		const kson::ByPulse<kson::CamPatternInvokeSwing> m_swingEvents;

		std::unordered_set<kson::Pulse> m_alreadyInvokedEventPulses;

		kson::Pulse m_startPulse = kPastPulse;
		kson::RelPulse m_durationRelPulse = kson::RelPulse{ 0 };
		int32 m_direction = 0;

	public:
		explicit CamPatternSwing(const kson::ChartData& chartData);

		void onLaserSlamJudged(kson::Pulse laserSlamPulse, int32 direction, kson::Pulse currentPulse);

		void applyToCamStatus(CamStatus& camStatusRef, kson::Pulse currentPulse) const;
	};
}
