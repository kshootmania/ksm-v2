﻿#include "audio_effect_main.hpp"
#include "audio_effect_utils.hpp"

namespace MusicGame::Audio
{
	namespace
	{
		constexpr double kLongFXNoteAudioEffectAutoPlaySec = 0.03;

		// TODO: 名前ではなくIDで管理したい(パフォーマンスのため)
		kson::FXLane<std::string> CreateLongFXNoteAudioEffectNames(const kson::ChartData& chartData)
		{
			const auto& longEvent = chartData.audio.audioEffect.fx.longEvent;
			kson::FXLane<std::string> convertedLongEvent;
			for (const auto& [audioEffectName, lanes] : longEvent)
			{
				for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
				{
					for (const auto& [y, _] : lanes[i])
					{
						convertedLongEvent[i].emplace(y, audioEffectName);
					}
				}
			}
			return convertedLongEvent;
		}

		kson::FXLane<ksmaudio::AudioEffect::ParamValueSetDict> CreateLongFXNoteAudioEffectParams(const kson::ChartData& chartData)
		{
			const auto& longEvent = chartData.audio.audioEffect.fx.longEvent;
			kson::FXLane<ksmaudio::AudioEffect::ParamValueSetDict> convertedLongEvent;
			for (const auto& [_, lanes] : longEvent)
			{
				for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
				{
					for (const auto& [y, dict] : lanes[i])
					{
						convertedLongEvent[i].emplace(y, ksmaudio::AudioEffect::StrDictToParamValueSetDict(dict));
					}
				}
			}
			return convertedLongEvent;
		}

		Optional<std::pair<kson::Pulse, kson::Interval>> CurrentLongNoteByTime(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse)
		{
			const auto currentNoteItr = kson::ValueItrAt(lane, currentPulse);
			if (currentNoteItr != lane.end())
			{
				const auto& [y, currentNote] = *currentNoteItr;
				if (y <= currentPulse && currentPulse < y + currentNote.length)
				{
					return *currentNoteItr;
				}
			}
			return none;
		}
	}

	void AudioEffectMain::registerAudioEffects(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
	{
		using AudioEffectUtils::PrecalculateUpdateTriggerTiming;

		const std::int64_t totalMeasures =
			kson::SecToMeasureIdx(bgm.durationSec(), chartData.beat, timingCache)
			+ 1/* 最後の小節の分を足す */
			+ 1/* インデックスを要素数にするために1を足す */;

		// FX
		for (const auto& [name, def] : chartData.audio.audioEffect.fx.def)
		{
			const auto& paramChangeDict = chartData.audio.audioEffect.fx.paramChange;
			const std::set<float> updateTriggerTiming =
				paramChangeDict.contains(name)
				? PrecalculateUpdateTriggerTiming(def, paramChangeDict.at(name), totalMeasures, chartData, timingCache)
				: PrecalculateUpdateTriggerTiming(def, totalMeasures, chartData, timingCache);

			bgm.emplaceAudioEffectFX(name, def, updateTriggerTiming);
		}

		// Laser
		for (const auto& [name, def] : chartData.audio.audioEffect.laser.def)
		{
			const auto& paramChangeDict = chartData.audio.audioEffect.laser.paramChange;
			const std::set<float> updateTriggerTiming =
				paramChangeDict.contains(name)
				? PrecalculateUpdateTriggerTiming(def, paramChangeDict.at(name), totalMeasures, chartData, timingCache)
				: PrecalculateUpdateTriggerTiming(def, totalMeasures, chartData, timingCache);

			bgm.emplaceAudioEffectLaser(name, def, updateTriggerTiming);
		}

		// テスト用
		// TODO: Remove this code
		{
			const kson::AudioEffectDef def = { .type = kson::AudioEffectType::Retrigger };
			const auto updateTriggerTiming = PrecalculateUpdateTriggerTiming(def, totalMeasures, chartData, timingCache);
			bgm.emplaceAudioEffectFX("retrigger", def, updateTriggerTiming);
		}
		{
			const kson::AudioEffectDef def = { .type = kson::AudioEffectType::Gate };
			const auto updateTriggerTiming = PrecalculateUpdateTriggerTiming(def, totalMeasures, chartData, timingCache);
			bgm.emplaceAudioEffectFX("gate", def, updateTriggerTiming);
		}
		{
			const kson::AudioEffectDef def = { .type = kson::AudioEffectType::Flanger };
			bgm.emplaceAudioEffectFX("flanger", def);
		}
		{
			const kson::AudioEffectDef def = { .type = kson::AudioEffectType::Bitcrusher };
			bgm.emplaceAudioEffectFX("bitcrusher", def);
		}
		{
			const kson::AudioEffectDef def = { .type = kson::AudioEffectType::Wobble };
			const auto updateTriggerTiming = PrecalculateUpdateTriggerTiming(def, totalMeasures, chartData, timingCache);
			bgm.emplaceAudioEffectFX("wobble", def, updateTriggerTiming);
		}
		{
			const kson::AudioEffectDef def = { .type = kson::AudioEffectType::Tapestop };
			bgm.emplaceAudioEffectFX("tapestop", def);
		}
	}

	kson::Dict<ksmaudio::AudioEffect::ParamValueSetDict> AudioEffectMain::currentActiveAudioEffectsFX(
		const std::array<Optional<std::pair<kson::Pulse, kson::Interval>>, kson::kNumFXLanesSZ>& currentLongNoteOfLanes, kson::Pulse currentPulseForAudio) const
	{
		kson::Dict<ksmaudio::AudioEffect::ParamValueSetDict> audioEffects; // TODO: 名前ではなくIDで管理したい(パフォーマンスのため)
		for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
		{
			// 2レーンのうち直近ロングFXノーツが押され始めた方のレーンを先に処理する
			// なお, Dict<T>::emplace()では同じキーの2回目の挿入は無視される
			static_assert(kson::kNumFXLanesSZ == 2U); // 下のコードはkNumFXLanesSZが2である前提である
			const std::size_t laneIdx = (i == 0) ? m_lastPressedLongFXNoteLaneIdx : (1U - m_lastPressedLongFXNoteLaneIdx);
			assert(laneIdx < kson::kNumFXLanesSZ);

			if (!currentLongNoteOfLanes[laneIdx].has_value())
			{
				continue;
			}

			const auto& [longNoteY, longNote] = *currentLongNoteOfLanes[laneIdx];
			const auto itr = kson::ValueItrAt(m_longFXNoteAudioEffectNames[laneIdx], currentPulseForAudio);
			if (itr == m_longFXNoteAudioEffectNames[laneIdx].end())
			{
				continue;
			}

			// 1本のロングFXノーツの途中で他の種類の音声エフェクトに変更される場合がある。
			// そのため、イベントが現在のロングノーツに属するかを調べるには、ロングノーツの開始点との一致判定ではなく、ロングノーツの範囲(開始～終了)の中にあるかを調べる必要がある。
			const auto& [longEventY, audioEffectName] = *itr;
			if (longEventY < longNoteY || longNoteY + longNote.length <= longEventY)
			{
				continue;
			}

			assert(m_longFXNoteAudioEffectParams[laneIdx].contains(longEventY));
			const auto& audioEffectParams = m_longFXNoteAudioEffectParams[laneIdx].at(longEventY);
			audioEffects.emplace(audioEffectName, audioEffectParams);
		}
		return audioEffects;
	}

	AudioEffectMain::AudioEffectMain(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
		: m_longFXNoteAudioEffectNames(CreateLongFXNoteAudioEffectNames(chartData))
		, m_longFXNoteAudioEffectParams(CreateLongFXNoteAudioEffectParams(chartData))
	{
		registerAudioEffects(bgm, chartData, timingCache);
	}

	void AudioEffectMain::update(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache, const AudioEffectInputStatus& inputStatus)
	{
		const double currentTimeSec = bgm.posSec();
		const double currentTimeSecForAudio = currentTimeSec + bgm.latencySec(); // Note: In BASS v2.4.13 and later, for unknown reasons, the effects are out of sync even after adding this latency.
		const kson::Pulse currentPulseForAudio = kson::SecToPulse(currentTimeSecForAudio, chartData.beat, timingCache);
		const double currentBPMForAudio = kson::TempoAt(currentPulseForAudio, chartData.beat);

		// ロングFXノーツの音声エフェクト
		bool bypassFX = true;
		std::array<Optional<std::pair<kson::Pulse, kson::Interval>>, kson::kNumFXLanesSZ> currentLongNoteOfLanes;
		for (std::size_t i = 0; i < kson::kNumFXLanesSZ; ++i)
		{
			const auto currentLongNoteByTime = CurrentLongNoteByTime(chartData.note.fx[i], currentPulseForAudio);

			// 音声エフェクトはバッファサイズによる遅延を回避するため、バッファサイズ分だけ早めに適用し始める必要がある。
			// これはプレイヤーのキー入力に関係なく実施する必要があるため、下記の「～.value_or(true)」で、longFXPressed[i]がnoneである(つまり判定中のロングFXノーツがまだ存在しない)場合でも、
			// あたかもロングFXが押されている(trueである)かのように扱っている。
			// 
			// それに加えて、キー入力がやや遅れた場合でも音声エフェクトが途切れないように、(バッファサイズとは別に)最初の30ms分はプレイヤーのキー入力に関係なく音声エフェクトを有効にしている。
			// 
			// HSP版での対応箇所: https://github.com/m4saka/kshootmania-v1-hsp/blob/19bfb6acbec8abd304b2e7dae6009df8e8e1f66f/src/scene/play/play_audio_effects.hsp#L488
			if (currentLongNoteByTime.has_value()
				&& (inputStatus.longFXPressed[i].value_or(true)
					|| (currentTimeSec - kson::PulseToSec(currentLongNoteByTime->first, chartData.beat, timingCache)) < kLongFXNoteAudioEffectAutoPlaySec))
			{
				currentLongNoteOfLanes[i] = currentLongNoteByTime;
				bypassFX = false;
				if (!m_longFXPressedPrev[i])
				{
					m_lastPressedLongFXNoteLaneIdx = i;
					m_longFXPressedPrev[i] = true;
				}
			}
			else
			{
				currentLongNoteOfLanes[i] = none;
				m_longFXPressedPrev[i] = false;
			}
		}
		const kson::Dict<ksmaudio::AudioEffect::ParamValueSetDict> activeAudioEffectsFX = currentActiveAudioEffectsFX(currentLongNoteOfLanes, currentPulseForAudio);
		bgm.updateAudioEffectFX(
			bypassFX,
			{
				.bpm = static_cast<float>(currentBPMForAudio),
				.sec = static_cast<float>(currentTimeSecForAudio),
			},
			activeAudioEffectsFX);

		// TODO: laser
	}
}
