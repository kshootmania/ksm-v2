#pragma once
#include "bgm.hpp"
#include "kson/chart_data.hpp"
#include "kson/util/timing_utils.hpp"

namespace MusicGame::Audio
{
	struct AudioEffectInputStatus
	{
		std::array<Optional<bool>, kson::kNumFXLanesSZ> longFXPressed;

		// LASERが音声エフェクト有効の前提条件を満たしているかどうか
		// (LASERがCRITICAL判定中、または、判定中のLASERノーツが存在しない場合にtrueになる)
		//
		// 判定中のLASERノーツが存在しない場合を加える必要があるのは、バッファサイズ分だけ先行したタイミングでエフェクトをかけ始める必要があるため
		std::array<bool, kson::kNumLaserLanesSZ> laserIsOnOrNone;
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
		const kson::ByPulse<Optional<AudioEffectInvocation>> m_laserPulseInvocations;

		std::array<bool, kson::kNumFXLanesSZ> m_longFXPressedPrev = { false, false };

		std::size_t m_lastPressedLongFXNoteLaneIdx = 0U;

		ksmaudio::AudioEffect::ActiveAudioEffectDict m_activeAudioEffectDictFX;

		void updateActiveAudioEffectDictFX(const std::array<Optional<std::pair<kson::Pulse, kson::Interval>>, kson::kNumFXLanesSZ>& currentLongNoteOfLanes, kson::Pulse currentPulseForAudio);

		// Siv3D非依存のksmaudioに渡す必要があるため、ここではOptionalではなくstd::optionalを使用(これらは暗黙変換されない)
		std::optional<std::size_t> getActiveLaserAudioEffectIdx(kson::Pulse currentPulseForAudio);
		
	public:
		AudioEffectMain(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache);

		void update(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache, const AudioEffectInputStatus& inputStatus);
	};
}
