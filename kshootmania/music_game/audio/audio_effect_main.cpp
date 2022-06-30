#include "audio_effect_main.hpp"
#include "audio_effect_utils.hpp"

namespace MusicGame::Audio
{
	namespace
	{
		constexpr double kLongFXNoteAudioEffectAutoPlaySec = 0.03;

		// TODO: Use IDs instead of names
		kson::FXLane<std::string> CreateLongFXNoteAudioEffectNames(const kson::ChartData& chartData)
		{
			const auto& longEvent = chartData.audio.audioEffect.fx.longEvent;
			kson::FXLane<std::string> convertedLongEvent;
			for (const auto& [audioEffectName, lanes] : longEvent)
			{
				for (std::size_t i = 0U; i < kson::kNumFXLanes; ++i)
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
				for (std::size_t i = 0U; i < kson::kNumFXLanes; ++i)
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
			const auto currentNoteItr = kson::CurrentAt(lane, currentPulse);
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
			kson::TimingUtils::SecToMeasureIdx(bgm.durationSec(), chartData.beat, timingCache)
			+ 1/* add last measure */
			+ 1/* index to size */;

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

		// Just for testing
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
	}

	kson::Dict<ksmaudio::AudioEffect::ParamValueSetDict> AudioEffectMain::currentActiveAudioEffectsFX(
		const std::array<Optional<std::pair<kson::Pulse, kson::Interval>>, kson::kNumFXLanes>& currentLongNoteOfLanes, kson::Pulse currentPulseForAudio) const
	{
		kson::Dict<ksmaudio::AudioEffect::ParamValueSetDict> audioEffects; // TODO: Use IDs instead of names
		for (std::size_t i = 0U; i < kson::kNumFXLanes; ++i)
		{
			// The first lane processed is the one where a long note was last pressed.
			// Note that Dict<T>::emplace() ignores a second insertion of the same key.
			static_assert(kson::kNumFXLanes == 2U);
			const std::size_t laneIdx = (i == 0) ? m_lastPressedLongFXNoteLaneIdx : (1U - m_lastPressedLongFXNoteLaneIdx); // This code assumes kNumFXLanes is 2
			assert(laneIdx < kson::kNumFXLanes);

			if (!currentLongNoteOfLanes[laneIdx].has_value())
			{
				continue;
			}

			const auto& [longNoteY, longNote] = *currentLongNoteOfLanes[laneIdx];
			const auto itr = kson::CurrentAt(m_longFXNoteAudioEffectNames[laneIdx], currentPulseForAudio);
			if (itr == m_longFXNoteAudioEffectNames[laneIdx].end())
			{
				continue;
			}

			// Note: Since multiple audio effect invocations (audio.audio_effect.fx.long_event) may be used within one long FX note, the determination of
			//       whether the event belongs to the current note is based on the range, not the start point.
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
		const kson::Pulse currentPulseForAudio = kson::TimingUtils::SecToPulse(currentTimeSecForAudio, chartData.beat, timingCache);
		const double currentBPMForAudio = kson::TimingUtils::PulseTempo(currentPulseForAudio, chartData.beat);

		// FX audio effects
		bool bypassFX = true;
		std::array<Optional<std::pair<kson::Pulse, kson::Interval>>, kson::kNumFXLanes> currentLongNoteOfLanes;
		for (std::size_t i = 0; i < kson::kNumFXLanes; ++i)
		{
			const auto currentLongNoteByTime = CurrentLongNoteByTime(chartData.note.fx[i], currentPulseForAudio);

			// Note: When longFXPressed[i] is none (i.e., there are no FX notes in actual time), it is treated as if a long FX note was pressed.
			//       This is because the audio effect should be activated ahead of time by the buffer size regardless of the input.
			//       In addition, the audio effect is autoplayed for 30 ms from the beginning of long notes so that the effect remains active even if the key press timing is slightly delayed.
			// Implementation in HSP: https://github.com/m4saka/kshootmania-v1-hsp/blob/19bfb6acbec8abd304b2e7dae6009df8e8e1f66f/src/scene/play/play_audio_effects.hsp#L488
			if (currentLongNoteByTime.has_value()
				&& (inputStatus.longFXPressed[i].value_or(true)
					|| (currentTimeSec - kson::TimingUtils::PulseToSec(currentLongNoteByTime->first, chartData.beat, timingCache)) < kLongFXNoteAudioEffectAutoPlaySec))
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
