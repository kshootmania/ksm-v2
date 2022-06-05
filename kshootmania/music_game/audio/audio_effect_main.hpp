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

		kson::Dict<ksmaudio::AudioEffect::ParamValueSetDict> currentActiveAudioEffectsFX(const kson::ChartData& chartData, const std::array<Optional<kson::Pulse>, kson::kNumFXLanes>& longNotePulseOfLanes) const;
		
	public:
		AudioEffectMain(const kson::ChartData& chartData);

		void update(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache, const AudioEffectInputStatus& inputStatus);
	};
}
