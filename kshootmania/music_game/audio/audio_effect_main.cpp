#include "audio_effect_main.hpp"

namespace MusicGame::Audio
{
	namespace
	{
		constexpr double kLongFXNoteAudioEffectAutoPlaySec = 0.03;

		// TODO: Use IDs instead of names
		kson::FXLane<std::string> CreateLongFXNoteAudioEffectNames(const kson::ChartData& chartData)
		{
			const auto& longEvent = chartData.audio.audioEffects.fx.longEvent;
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
			const auto& longEvent = chartData.audio.audioEffects.fx.longEvent;
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

		Optional<kson::Pulse> CurrentLongNotePulseByTime(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse)
		{
			const auto currentNoteItr = kson::CurrentAt(lane, currentPulse);
			if (currentNoteItr != lane.end())
			{
				const auto& [y, currentNote] = *currentNoteItr;
				if (y <= currentPulse && currentPulse < y + currentNote.length)
				{
					return y;
				}
			}
			return none;
		}
	}

	kson::Dict<ksmaudio::AudioEffect::ParamValueSetDict> AudioEffectMain::currentActiveAudioEffectsFX(
		const std::array<Optional<kson::Pulse>, kson::kNumFXLanes>& longNotePulseOfLanes) const
	{
		kson::Dict<ksmaudio::AudioEffect::ParamValueSetDict> audioEffects;
		for (std::size_t i = 0U; i < kson::kNumFXLanes; ++i)
		{
			// The first lane processed is the one where a long note was last pressed.
			// Note that Dict<T>::emplace() ignores a second insertion of the same key.
			static_assert(kson::kNumFXLanes == 2U);
			const std::size_t laneIdx = (i == 0) ? m_lastPressedLongFXNoteLaneIdx : (1U - m_lastPressedLongFXNoteLaneIdx); // This code assumes kNumFXLanes is 2
			assert(laneIdx < kson::kNumFXLanes);

			if (!longNotePulseOfLanes[laneIdx].has_value())
			{
				continue;
			}

			const kson::Pulse y = *longNotePulseOfLanes[laneIdx];
			if (m_longFXNoteAudioEffectNames[laneIdx].contains(y))
			{
				assert(m_longFXNoteAudioEffectParams.contains(y));
				audioEffects.emplace(m_longFXNoteAudioEffectNames[laneIdx].at(y), m_longFXNoteAudioEffectParams[laneIdx].at(y));
			}
		}
		return audioEffects;
	}

	AudioEffectMain::AudioEffectMain(const kson::ChartData& chartData)
		: m_longFXNoteAudioEffectNames(CreateLongFXNoteAudioEffectNames(chartData))
		, m_longFXNoteAudioEffectParams(CreateLongFXNoteAudioEffectParams(chartData))
	{
	}

	void AudioEffectMain::update(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache, const AudioEffectInputStatus& inputStatus)
	{
		const double currentTimeSec = bgm.posSec();
		const double currentTimeSecForAudio = currentTimeSec + bgm.latencySec(); // Note: In BASS v2.4.13 and later, for unknown reasons, the effects are out of sync even after adding this latency.
		const kson::Pulse currentPulseForAudio = kson::TimingUtils::MsToPulse(MathUtils::SecToMs(currentTimeSecForAudio), chartData.beat, timingCache);
		const double currentBPMForAudio = kson::TimingUtils::PulseTempo(currentPulseForAudio, chartData.beat);

		// FX audio effects
		std::array<Optional<kson::Pulse>, kson::kNumFXLanes> currentLongNotePulseOfLanes;
		for (std::size_t i = 0; i < kson::kNumFXLanes; ++i)
		{
			const Optional<kson::Pulse> currentLongNotePulseByTime = CurrentLongNotePulseByTime(chartData.note.fxLanes[i], currentPulseForAudio);

			const double timeSecSinceLongNoteStart =
				currentLongNotePulseByTime.has_value()
					? (currentTimeSec - MathUtils::MsToSec(kson::TimingUtils::PulseToMs(*currentLongNotePulseByTime, chartData.beat, timingCache)))
					: 0.0;

			// Note: When longFXPressed[i] is none (i.e., there are no FX notes in actual time), it is treated as if a long FX note was pressed.
			//       This is because the audio effect should be activated ahead of time by the buffer size + 30ms regardless of the input.
			// Implementation in HSP: https://github.com/m4saka/kshootmania-v1-hsp/blob/19bfb6acbec8abd304b2e7dae6009df8e8e1f66f/src/scene/play/play_audio_effects.hsp#L488
			if (currentLongNotePulseByTime.has_value() && (inputStatus.longFXPressed[i].value_or(true) || (timeSecSinceLongNoteStart < kLongFXNoteAudioEffectAutoPlaySec)))
			{
				currentLongNotePulseOfLanes[i] = currentLongNotePulseByTime;
				if (!m_longFXPressedPrev[i])
				{
					m_lastPressedLongFXNoteLaneIdx = i;
					m_longFXPressedPrev[i] = true;
				}
			}
			else
			{
				currentLongNotePulseOfLanes[i] = none;
				m_longFXPressedPrev[i] = false;
			}
		}
		const kson::Dict<ksmaudio::AudioEffect::ParamValueSetDict> activeAudioEffectsFX = currentActiveAudioEffectsFX(currentLongNotePulseOfLanes);
		bgm.updateAudioEffectFX({
			.bpm = static_cast<float>(currentBPMForAudio),
			.sec = static_cast<float>(currentTimeSecForAudio),
		}, activeAudioEffectsFX);

		// TODO: laser
	}
}
