#pragma once
#include "audio_effect_update_info.hpp"
#include "bgm.hpp"
#include "kson/util/timing_utils.hpp"

namespace MusicGame::Audio
{
	class AudioEffectMain
	{
	private:
		AudioEffectUpdateInfo m_audioEffectUpdateInfo;

	public:
		AudioEffectMain(const kson::ChartData& chartData);

		void update(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache);
	};
}
