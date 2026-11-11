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

	// v1で音声エフェクトをかけないLASERセクション末尾の区間
	using LegacyLaserTailIntervals = kson::LaserLane<kson::Interval>;

	/// @brief v1で音声エフェクトをかけないLASERセクション末尾の区間を取得
	/// @param chartData 譜面データ(LASERの曲線を展開する前のものを渡す)
	/// @return 区間(ver=200以降の譜面の場合は空)
	LegacyLaserTailIntervals MakeLegacyLaserTailIntervals(const kson::ChartData& chartData);

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

		/// @param fxNoteLanesForAudio 音声エフェクト用のFXノーツ(S-RANDOM時は配置適用前のFXノーツを渡す)
		/// @param legacyLaserTailIntervals v1で音声エフェクトをかけないLASERセクション末尾の区間
		void update(BGM& bgm, const kson::ChartData& chartData, const kson::FXLane<kson::Interval>& fxNoteLanesForAudio, const LegacyLaserTailIntervals& legacyLaserTailIntervals, const kson::TimingCache& timingCache, const AudioEffectInputStatus& inputStatus, kson::Pulse currentPulseForSwitchAudio);
	};
}
