#include "audio_effect_main.hpp"

namespace MusicGame::Audio
{
	namespace
	{
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

		std::array<Optional<kson::Pulse>, kson::kNumFXLanes> CurrentLongNotePulseOfLanesByTime(const kson::ChartData& chartData, kson::Pulse time)
		{
			std::array<Optional<kson::Pulse>, kson::kNumFXLanes> pulses;
			for (std::size_t i = 0U; i < kson::kNumFXLanes; ++i)
			{
				pulses[i] = CurrentLongNotePulseByTime(chartData.note.fxLanes[i], time);
			}
			return pulses;
		}

		// costly?
		std::set<std::string> CurrentAudioEffectNamesFX(const kson::ChartData& chartData, const std::array<Optional<kson::Pulse>, kson::kNumFXLanes>& longNotePulseOfLanes)
		{
			std::set<std::string> set;
			for (std::size_t i = 0U; i < kson::kNumFXLanes; ++i)
			{
				if (!longNotePulseOfLanes[i].has_value())
				{
					continue;
				}

				for (const auto& [audioEffectName, map] : chartData.audio.audioEffects.fx.longEvent)
				{
					if (map[i].contains(*longNotePulseOfLanes[i]))
					{
						set.insert(audioEffectName);
					}
				}
			}

			return set;
		}
	}

	AudioEffectMain::AudioEffectMain(const kson::ChartData& chartData)
	{
	}

	void AudioEffectMain::update(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
	{
		const double currentTimeSec = bgm.posSec();
		const double currentTimeSecForAudio = currentTimeSec + bgm.latencySec(); // Note: Not sure why, but the effect is out of sync with BASS v2.4.13 or later (;_;)
		const kson::Pulse currentPulseForAudio = kson::TimingUtils::MsToPulse(MathUtils::SecToMs(currentTimeSecForAudio), chartData.beat, timingCache);
		const double currentBPMForAudio = kson::TimingUtils::PulseTempo(currentPulseForAudio, chartData.beat);

		const std::array<Optional<kson::Pulse>, kson::kNumFXLanes> currentLongNotePulseOfLanes = CurrentLongNotePulseOfLanesByTime(chartData, currentPulseForAudio);
		const std::set<std::string> currentAudioEffectNamesFX = CurrentAudioEffectNamesFX(chartData, currentLongNotePulseOfLanes);
		bgm.updateAudioEffectFX({
			.v = 0.0f,
			.bpm = static_cast<float>(currentBPMForAudio),
			.sec = static_cast<float>(currentTimeSecForAudio),
		}, currentAudioEffectNamesFX);
		// TODO: laser
	}
}
