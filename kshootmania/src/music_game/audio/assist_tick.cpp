#include "assist_tick.hpp"
#include "audio_defines.hpp"

namespace MusicGame::Audio
{
	namespace
	{
		// Note: ByPulse<Interval>はstd::map<Pulse, Interval>の型エイリアス
		kson::Pulse CurrentNotePulse(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse)
		{
			const auto nextNote = lane.upper_bound(currentPulse);
			if (nextNote == lane.begin())
			{
				return MusicGame::kPastPulse;
			}

			return std::prev(nextNote)->first;
		}
	}

	AssistTick::AssistTick(bool enabled)
		: m_enabled(enabled)
		, m_btTickSound("se/tick.wav")
		, m_fxTickSound("se/tick2.wav")
	{
	}

	void AssistTick::update(const kson::ChartData& chartData, const kson::TimingCache& timingCache, double currentTimeSec)
	{
		if (!m_enabled)
		{
			return;
		}

		const kson::Pulse currentPulseForAssistTick = kson::SecToPulse(currentTimeSec + kSELatencySec, chartData.beat, timingCache);

		// BT
		for (std::size_t i = 0; i < kson::kNumBTLanesSZ; ++i)
		{
			const kson::Pulse currentNotePulse = CurrentNotePulse(chartData.note.bt[i], currentPulseForAssistTick);
			if (currentNotePulse > m_btPlayedPulses[i])
			{
				// BTの場合はチップノーツとロングノーツの両方で鳴らす
				m_btTickSound.play();
				m_btPlayedPulses[i] = currentNotePulse;
			}
		}

		// FX
		for (std::size_t i = 0; i < kson::kNumFXLanesSZ; ++i)
		{
			const kson::Pulse currentNotePulse = CurrentNotePulse(chartData.note.fx[i], currentPulseForAssistTick);
			if (currentNotePulse > m_fxPlayedPulses[i])
			{
				if (chartData.note.fx[i].contains(currentNotePulse) && chartData.note.fx[i].at(currentNotePulse).length == 0)
				{
					// FXの場合はチップノーツのみ鳴らす
					m_fxTickSound.play();
				}
				m_fxPlayedPulses[i] = currentNotePulse;
			}
		}
	}
}
