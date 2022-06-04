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

		Optional<kson::Pulse> CurrentLongNotePulseByTime(const kson::ByPulse<kson::Interval>& lane, kson::Pulse time)
		{
			const auto currentNoteItr = kson::CurrentAt(lane, time);
			if (currentNoteItr != lane.end())
			{
				const auto& [y, currentNote] = *currentNoteItr;
				if (y <= time && time < y + currentNote.length)
				{
					return y;
				}
			}
			return none;
		}

		std::array<Optional<kson::Pulse>, kson::kNumFXLanes> CurrentLongFXNotePulseOfLanesByTime(const kson::ChartData& chartData, kson::Pulse time)
		{
			std::array<Optional<kson::Pulse>, kson::kNumFXLanes> pulses;
			for (std::size_t i = 0U; i < kson::kNumFXLanes; ++i)
			{
				pulses[i] = CurrentLongNotePulseByTime(chartData.note.fxLanes[i], time);
			}
			return pulses;
		}
	}

	std::set<std::string> AudioEffectMain::currentAudioEffectNamesFX(const kson::ChartData& chartData, const std::array<Optional<kson::Pulse>, kson::kNumFXLanes>& longNotePulseOfLanes) const
	{
		std::set<std::string> set;
		for (std::size_t i = 0U; i < kson::kNumFXLanes; ++i)
		{
			if (!longNotePulseOfLanes[i].has_value())
			{
				continue;
			}

			const kson::Pulse y = *longNotePulseOfLanes[i];
			if (m_longFXNoteAudioEffectNames[i].contains(y))
			{
				set.emplace(m_longFXNoteAudioEffectNames[i].at(y));
			}
		}

		return set;
	}

	AudioEffectMain::AudioEffectMain(const kson::ChartData& chartData)
		: m_longFXNoteAudioEffectNames(CreateLongFXNoteAudioEffectNames(chartData))
		, m_longFXNoteAudioEffectParams(CreateLongFXNoteAudioEffectParams(chartData))
	{
	}

	void AudioEffectMain::update(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
	{
		const double currentTimeSec = bgm.posSec();
		const double currentTimeSecForAudio = currentTimeSec + bgm.latencySec(); // Note: Not sure why, but the effect is out of sync with BASS v2.4.13 or later (;_;)
		const kson::Pulse currentPulseForAudio = kson::TimingUtils::MsToPulse(MathUtils::SecToMs(currentTimeSecForAudio), chartData.beat, timingCache);
		const double currentBPMForAudio = kson::TimingUtils::PulseTempo(currentPulseForAudio, chartData.beat);

		const std::array<Optional<kson::Pulse>, kson::kNumFXLanes> currentLongNotePulseOfLanes = CurrentLongFXNotePulseOfLanesByTime(chartData, currentPulseForAudio);
		const std::set<std::string> audioEffectNames = currentAudioEffectNamesFX(chartData, currentLongNotePulseOfLanes);
		bgm.updateAudioEffectFX({
			.v = 0.0f,
			.bpm = static_cast<float>(currentBPMForAudio),
			.sec = static_cast<float>(currentTimeSecForAudio),
		}, audioEffectNames);
		// TODO: laser
	}
}
