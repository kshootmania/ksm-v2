#include "audio_effect_main.hpp"

namespace MusicGame::Audio
{
	namespace
	{
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

	kson::Dict<ksmaudio::AudioEffect::ParamValueSetDict> AudioEffectMain::currentActiveAudioEffectsFX(const kson::ChartData& chartData, const std::array<Optional<kson::Pulse>, kson::kNumFXLanes>& longNotePulseOfLanes) const
	{
		kson::Dict<ksmaudio::AudioEffect::ParamValueSetDict> audioEffects;
		for (std::size_t i = 0U; i < kson::kNumFXLanes; ++i)
		{
			if (!longNotePulseOfLanes[i].has_value())
			{
				continue;
			}

			const kson::Pulse y = *longNotePulseOfLanes[i];
			if (m_longFXNoteAudioEffectNames[i].contains(y))
			{
				assert(m_longFXNoteAudioEffectParams.contains(y));
				audioEffects.emplace(m_longFXNoteAudioEffectNames[i].at(y), m_longFXNoteAudioEffectParams[i].at(y));
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
			// Note: When longFXPressed[i].has_value() is false (i.e., there are no FX notes in actual time), it is treated as if a long FX note was pressed.
			//       This is because the audio effect must be activated ahead of time by the buffer size, and the effect must be activated regardless of
			//       the input when a FX note has not yet arrived.
			if (inputStatus.longFXPressed[i].has_value() && !*inputStatus.longFXPressed[i])
			{
				currentLongNotePulseOfLanes[i] = none;
			}
			else
			{
				currentLongNotePulseOfLanes[i] = CurrentLongNotePulseByTime(chartData.note.fxLanes[i], currentPulseForAudio);
			}
		}
		const kson::Dict<ksmaudio::AudioEffect::ParamValueSetDict> activeAudioEffectsFX = currentActiveAudioEffectsFX(chartData, currentLongNotePulseOfLanes);
		bgm.updateAudioEffectFX({
			.bpm = static_cast<float>(currentBPMForAudio),
			.sec = static_cast<float>(currentTimeSecForAudio),
		}, activeAudioEffectsFX);

		// TODO: laser
	}
}
