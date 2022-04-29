#include "button_lane_judgment.hpp"

using namespace MusicGame::Judgment;

namespace
{
	constexpr double kHalveComboBPMThreshold = 256.0;

	std::map<ksh::Pulse, double> CreatePulseToSec(const ksh::ByPulse<ksh::Interval>& lane, const ksh::BeatMap& beatMap, const ksh::TimingCache& timingCache)
	{
		std::map<ksh::Pulse, double> pulseToSec;

		for (const auto& [y, note] : lane)
		{
			if (!pulseToSec.contains(y))
			{
				const double sec = MathUtils::MsToSec(ksh::TimingUtils::PulseToMs(y, beatMap, timingCache));
				pulseToSec.emplace(y, sec);
			}
			if (!pulseToSec.contains(y + note.length))
			{
				const double sec = MathUtils::MsToSec(ksh::TimingUtils::PulseToMs(y + note.length, beatMap, timingCache));
				pulseToSec.emplace(y + note.length, sec);
			}
		}

		return pulseToSec;
	}

	ksh::ByPulse<JudgmentResult> CreateChipNoteJudgmentArray(const ksh::ByPulse<ksh::Interval>& lane)
	{
		ksh::ByPulse<JudgmentResult> judgmentArray;

		for (const auto& [y, note] : lane)
		{
			if (note.length == 0)
			{
				judgmentArray.emplace(y, JudgmentResult::kUnspecified);
			}
		}

		return judgmentArray;
	}

	ksh::ByPulse<JudgmentResult> CreateLongNoteJudgmentArray(const ksh::ByPulse<ksh::Interval>& lane, const ksh::BeatMap& beatMap)
	{
		const ksh::Pulse unitMeasure = beatMap.resolution * 4;

		ksh::ByPulse<JudgmentResult> judgmentArray;

		for (const auto& [y, note] : lane) // TODO: merge two long notes if note1.y + note1.l == note2.y
		{
			if (note.length > 0)
			{
				// Determine whether to halve the combo based on the BPM at the start of the note
				// (BPM changes during the notes are ignored)
				const bool halvesCombo = ksh::TimingUtils::PulseTempo(y, beatMap) >= kHalveComboBPMThreshold;
				const ksh::RelPulse minPulseInterval = halvesCombo ? (unitMeasure * 3 / 8) : (unitMeasure * 3 / 16);
				const ksh::RelPulse pulseInterval = halvesCombo ? (unitMeasure / 8) : (unitMeasure / 16);

				if (note.length <= minPulseInterval)
				{
					judgmentArray.emplace(y, JudgmentResult::kUnspecified);
				}
				else
				{
					const ksh::Pulse start = ((y + pulseInterval - 1) / pulseInterval + 1) * pulseInterval;
					const ksh::Pulse end = y + note.length - pulseInterval;

					for (ksh::Pulse pulse = start; pulse < end; pulse += pulseInterval)
					{
						judgmentArray.emplace(pulse, JudgmentResult::kUnspecified);
					}
				}
			}
		}

		return judgmentArray;
	}
}

Optional<KeyBeamType> ButtonLaneJudgment::processKeyDown(const ksh::ByPulse<ksh::Interval>& lane, ksh::Pulse currentPulse, double currentTimeSec)
{
	using namespace TimingWindow;

	// Pick up the nearest note from the lane
	bool found = false;
	double minDistance;
	ksh::Pulse nearestNotePulse;
	for (auto itr = lane.upper_bound(m_passedNotePulse); itr != lane.end(); ++itr)
	{
		const auto& [y, note] = *itr;
		const double sec = m_pulseToSec.at(y);
		const double endSec = (note.length == 0) ? sec : m_pulseToSec.at(y + note.length);
		if (currentTimeSec - endSec >= ChipNote::kWindowSecError)
		{
			m_passedNotePulse = y;
			continue;
		}

		if (note.length == 0) // Chip note
		{
			if (m_chipJudgmentArray.at(y) != JudgmentResult::kUnspecified)
			{
				continue;
			}

			if (!found || Abs(sec - currentTimeSec) < minDistance)
			{
				nearestNotePulse = y;
				minDistance = Abs(sec - currentTimeSec);
				found = true;
			}
			else if (found && Abs(sec - currentTimeSec) >= minDistance && y > currentPulse)
			{
				break;
			}
		}
		else // Long note
		{
			if ((!found || Abs(sec - currentTimeSec) < minDistance) && sec - currentTimeSec <= LongNote::kWindowSecPreHold && (y + note.length > currentPulse))
			{
				m_currentLongNotePulse = y;
				return none;
			}
			else if (found && sec - currentTimeSec > LongNote::kWindowSecPreHold && y > currentPulse)
			{
				break;
			}
		}
	}

	if (found)
	{
		if (minDistance < ChipNote::kWindowSecCritical)
		{
			m_chipJudgmentArray.at(nearestNotePulse) = JudgmentResult::kCritical;
			m_scoreValue += kScoreValueCritical;
			return KeyBeamType::kCritical;
		}
		else if (minDistance < ChipNote::kWindowSecNear)
		{
			m_chipJudgmentArray.at(nearestNotePulse) = JudgmentResult::kNear;
			m_scoreValue += kScoreValueNear;
			return KeyBeamType::kNear;
		}
		else if (minDistance < ChipNote::kWindowSecError) // TODO: easy gauge, fast/slow
		{
			m_chipJudgmentArray.at(nearestNotePulse) = JudgmentResult::kError;
			return KeyBeamType::kDefault;
		}
	}

	return KeyBeamType::kDefault;
}

void MusicGame::Judgment::ButtonLaneJudgment::processKeyPressed(const ksh::ByPulse<ksh::Interval>& lane, ksh::Pulse currentPulse, double currentSec)
{
	if (m_currentLongNotePulse.has_value())
	{
		const ksh::Pulse noteStartPulse = *m_currentLongNotePulse;
		const ksh::Pulse noteEndPulse = *m_currentLongNotePulse + lane.at(*m_currentLongNotePulse).length;
		const ksh::Pulse limitPulse = Min(currentPulse, noteEndPulse);

		for (auto itr = m_longJudgmentArray.upper_bound(Max(noteStartPulse, m_prevPulse) - 1); itr != m_longJudgmentArray.end(); ++itr)
		{
			auto& [y, result] = *itr;
			if (y >= limitPulse)
			{
				break;
			}

			if (result == JudgmentResult::kUnspecified)
			{
				result = JudgmentResult::kCritical;
				m_scoreValue += kScoreValueCritical;
			}
		}
	}
}

ButtonLaneJudgment::ButtonLaneJudgment(KeyConfig::Button keyConfigButton, const ksh::ByPulse<ksh::Interval>& lane, const ksh::BeatMap& beatMap, const ksh::TimingCache& timingCache)
	: m_keyConfigButton(keyConfigButton)
	, m_pulseToSec(CreatePulseToSec(lane, beatMap, timingCache))
	, m_chipJudgmentArray(CreateChipNoteJudgmentArray(lane))
	, m_longJudgmentArray(CreateLongNoteJudgmentArray(lane, beatMap))
	, m_scoreValueMax(static_cast<int32>(m_chipJudgmentArray.size() + m_longJudgmentArray.size()) * kScoreValueCritical)
{
}

void MusicGame::Judgment::ButtonLaneJudgment::update(const ksh::ByPulse<ksh::Interval>& lane, ksh::Pulse currentPulse, double currentTimeSec, Graphics::LaneState& laneStateRef)
{
	// Chip note & long note start
	if (KeyConfig::Down(m_keyConfigButton))
	{
		const Optional<Judgment::KeyBeamType> keyBeamType = processKeyDown(lane, currentPulse, currentTimeSec);
		if (keyBeamType.has_value())
		{
			laneStateRef.keyBeamTimeSec = currentTimeSec;
			laneStateRef.keyBeamType = *keyBeamType;
		}
	}

	// Long note hold
	if (KeyConfig::Pressed(m_keyConfigButton))
	{
		processKeyPressed(lane, currentPulse, currentTimeSec);
	}

	// Long note release
	if (m_currentLongNotePulse.has_value() && 
		(KeyConfig::Up(m_keyConfigButton) || (*m_currentLongNotePulse + lane.at(*m_currentLongNotePulse).length < currentPulse)))
	{
		m_currentLongNotePulse = none;
	}

	laneStateRef.currentLongNotePulse = m_currentLongNotePulse;
	m_prevPulse = currentPulse;
}

int32 MusicGame::Judgment::ButtonLaneJudgment::scoreValue() const
{
	return m_scoreValue;
}

int32 MusicGame::Judgment::ButtonLaneJudgment::scoreValueMax() const
{
	return m_scoreValueMax;
}
