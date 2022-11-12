#pragma once
#include "bgm.hpp"
#include "kson/chart_data.hpp"
#include "kson/util/timing_utils.hpp"

namespace MusicGame::Audio
{
	struct AudioEffectInputStatus
	{
		std::array<Optional<bool>, kson::kNumFXLanesSZ> longFXPressed;

		std::array<Optional<float>, kson::kNumLaserLanesSZ> laserValues;
	};

	struct AudioEffectInvocation
	{
		std::size_t audioEffectIdx;

		ksmaudio::AudioEffect::ParamValueSetDict overrideParams;
	};

	class AudioEffectMain
	{
	private:
		const kson::FXLane<AudioEffectInvocation> m_longFXNoteInvocations;

		std::array<bool, kson::kNumFXLanesSZ> m_longFXPressedPrev = { false, false };

		std::size_t m_lastPressedLongFXNoteLaneIdx = 0U;

		ksmaudio::AudioEffect::ActiveAudioEffectDict m_activeAudioEffectDictFX;

		void updateActiveAudioEffectDictFX(const std::array<Optional<std::pair<kson::Pulse, kson::Interval>>, kson::kNumFXLanesSZ>& currentLongNoteOfLanes, kson::Pulse currentPulseForAudio);
		
	public:
		AudioEffectMain(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache);

		void update(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache, const AudioEffectInputStatus& inputStatus);
	};
}
