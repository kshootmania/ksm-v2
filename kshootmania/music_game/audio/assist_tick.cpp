#include "assist_tick.hpp"

namespace
{
	// Assist tick sound is played 30ms early
	constexpr double kLatencySec = 0.03;

	// Note: ByPulse<Interval> is std::map<Pulse, Interval>
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

MusicGame::Audio::AssistTick::AssistTick(bool enabled)
	: m_enabled(enabled)
	, m_btTickSound(U"se/tick.wav")
	, m_fxTickSound(U"se/tick2.wav")
{
}

void MusicGame::Audio::AssistTick::update(const kson::ChartData& chartData, const kson::TimingCache& timingCache, double currentTimeSec)
{
	if (!m_enabled)
	{
		return;
	}

	const kson::Pulse currentPulseForAssistTick = kson::TimingUtils::MsToPulse(MathUtils::SecToMs(currentTimeSec + kLatencySec), chartData.beat, timingCache);

	// BT notes
	for (std::size_t i = 0; i < kson::kNumBTLanes; ++i)
	{
		const kson::Pulse currentNotePulse = CurrentNotePulse(chartData.note.btLanes[i], currentPulseForAssistTick);
		if (currentNotePulse > m_btPlayedPulses[i])
		{
			// Chip & long notes
			m_btTickSound.stop();
			m_btTickSound.play();
			m_btPlayedPulses[i] = currentNotePulse;
		}
	}

	// FX notes
	for (std::size_t i = 0; i < kson::kNumFXLanes; ++i)
	{
		const kson::Pulse currentNotePulse = CurrentNotePulse(chartData.note.fxLanes[i], currentPulseForAssistTick);
		if (currentNotePulse > m_fxPlayedPulses[i])
		{
			if (chartData.note.fxLanes[i].contains(currentNotePulse) && chartData.note.fxLanes[i].at(currentNotePulse).length == 0)
			{
				// Chip notes only
				m_fxTickSound.stop();
				m_fxTickSound.play();
			}
			m_fxPlayedPulses[i] = currentNotePulse;
		}
	}
}
