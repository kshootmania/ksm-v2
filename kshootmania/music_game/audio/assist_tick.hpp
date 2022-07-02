#pragma once
#include "music_game/game_defines.hpp"
#include "kson/chart_data.hpp"
#include "kson/util/timing_utils.hpp"

namespace MusicGame::Audio
{
	class AssistTick
	{
	private:
		bool m_enabled;
		s3d::Audio m_btTickSound;
		s3d::Audio m_fxTickSound;

		std::array<kson::Pulse, kson::kNumBTLanesSZ> m_btPlayedPulses = { kPastPulse, kPastPulse, kPastPulse, kPastPulse };
		std::array<kson::Pulse, kson::kNumFXLanesSZ> m_fxPlayedPulses = { kPastPulse, kPastPulse };

	public:
		explicit AssistTick(bool enabled);

		void update(const kson::ChartData& chartData, const kson::TimingCache& timingCache, double currentTimeSec);
	};
}
