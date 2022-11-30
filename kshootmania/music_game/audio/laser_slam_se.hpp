#pragma once
#include "music_game/game_defines.hpp"
#include "music_game/game_status.hpp"
#include "kson/chart_data.hpp"
#include "kson/util/timing_utils.hpp"
#include "ksmaudio/ksmaudio.hpp"

namespace MusicGame::Audio
{
	class LaserSlamSE
	{
	private:
		ksmaudio::Sample m_sample;

		std::array<double, kson::kNumLaserLanesSZ> m_lastPlayedTimeSecs = { kPastTimeSec, kPastTimeSec };

	public:
		explicit LaserSlamSE(const kson::ChartData& chartData, const kson::TimingCache& timingCache);

		void update(const GameStatus& gameStatus);
	};
}
