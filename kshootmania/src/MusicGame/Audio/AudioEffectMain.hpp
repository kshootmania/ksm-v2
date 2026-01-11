#pragma once
#include "BGM.hpp"
#include "kson/ChartData.hpp"
#include "kson/Util/TimingUtils.hpp"

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

	struct DSPAudioEffectInvocation
	{
		std::size_t audioEffectIdx;

		// longEventの上書きパラメータ
		ksmaudio::AudioEffect::ParamValueSetDict overrideParams;

		// LASER音声エフェクトの種類がpeaking_filterかどうか
		// (FXでは不使用。LASER音声エフェクトでのpeaking_filter_delayを適用するかどうかに使用)
		bool isPeakingFilterLaser = false;
	};

	struct SwitchAudioInvocation
	{
		std::size_t switchAudioIdx;
	};

	using AudioEffectInvocation = std::variant<DSPAudioEffectInvocation, SwitchAudioInvocation>;

	class AudioEffectMain
	{
	private:
		const kson::FXLane<Optional<AudioEffectInvocation>> m_longFXNoteInvocations;
		const kson::ByPulse<Optional<AudioEffectInvocation>> m_laserPulseInvocations;

		const double m_audioProcDelaySec;
		const double m_peakingFilterDelaySec;

		std::array<bool, kson::kNumFXLanesSZ> m_longFXPressedPrev = { false, false };

		std::size_t m_lastPressedLongFXNoteLaneIdx = 0U;

		ksmaudio::AudioEffect::ActiveAudioEffectDict m_activeAudioEffectDictFX;

		void updateActiveAudioEffectDictFX(const std::array<Optional<std::pair<kson::Pulse, kson::Interval>>, kson::kNumFXLanesSZ>& currentLongNoteOfLanes, kson::Pulse currentPulseForAudio);

		const Optional<AudioEffectInvocation>& getActiveLaserAudioEffectInvocation(kson::Pulse currentPulseForAudio) const;
		
	public:
		AudioEffectMain(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache, const FilePath& parentPath, double audioProcDelaySec, double bgmVolume);

		void update(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache, const AudioEffectInputStatus& inputStatus, kson::Pulse currentPulseForSwitchAudio);
	};
}
