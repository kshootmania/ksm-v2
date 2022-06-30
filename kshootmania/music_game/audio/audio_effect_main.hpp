#pragma once
#include "bgm.hpp"
#include "kson/chart_data.hpp"
#include "kson/util/timing_utils.hpp"

namespace MusicGame::Audio
{
	struct AudioEffectInputStatus
	{
		std::array<Optional<bool>, kson::kNumFXLanes> longFXPressed;

		std::array<Optional<float>, kson::kNumLaserLanes> laserValues;
	};

	class AudioEffectMain
	{
	private:
		const kson::FXLane<std::string> m_longFXNoteAudioEffectNames;
		const kson::FXLane<ksmaudio::AudioEffect::ParamValueSetDict> m_longFXNoteAudioEffectParams;

		std::array<bool, kson::kNumFXLanes> m_longFXPressedPrev = { false, false };
		std::size_t m_lastPressedLongFXNoteLaneIdx = 0U;

		void registerAudioEffects(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache);

		kson::Dict<ksmaudio::AudioEffect::ParamValueSetDict> currentActiveAudioEffectsFX(
			const std::array<Optional<std::pair<kson::Pulse, kson::Interval>>, kson::kNumFXLanes>& longNoteOfLanes, kson::Pulse currentPulseForAudio) const;
		
	public:
		AudioEffectMain(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache);

		void update(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache, const AudioEffectInputStatus& inputStatus);
	};
}
