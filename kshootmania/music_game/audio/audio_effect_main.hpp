#pragma once
#include "bgm.hpp"
#include "kson/chart_data.hpp"
#include "kson/util/timing_utils.hpp"

namespace MusicGame::Audio
{
	class AudioEffectMain
	{
	public:
		AudioEffectMain(const kson::ChartData& chartData);

		void update(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache);
	};
}
