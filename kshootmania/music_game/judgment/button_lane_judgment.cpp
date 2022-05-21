#include "button_lane_judgment.hpp"

using namespace MusicGame::Judgment;

namespace
{
	constexpr double kHalveComboBPMThreshold = 256.0;

	std::map<kson::Pulse, double> CreatePulseToSec(const kson::ByPulse<kson::Interval>& lane, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache)
	{
		std::map<kson::Pulse, double> pulseToSec;

		for (const auto& [y, note] : lane)
		{
			if (!pulseToSec.contains(y))
			{
				const double sec = MathUtils::MsToSec(kson::TimingUtils::PulseToMs(y, beatInfo, timingCache));
				pulseToSec.emplace(y, sec);
			}
			if (!pulseToSec.contains(y + note.length))
			{
				const double sec = MathUtils::MsToSec(kson::TimingUtils::PulseToMs(y + note.length, beatInfo, timingCache));
				pulseToSec.emplace(y + note.length, sec);
			}
		}

		return pulseToSec;
	}

	kson::ByPulse<JudgmentResult> CreateChipNoteJudgmentArray(const kson::ByPulse<kson::Interval>& lane)
	{
		kson::ByPulse<JudgmentResult> judgmentArray;

		for (const auto& [y, note] : lane)
		{
			if (note.length == 0)
			{
				judgmentArray.emplace(y, JudgmentResult::kUnspecified);
			}
		}

		return judgmentArray;
	}

	kson::ByPulse<JudgmentResult> CreateLongNoteJudgmentArray(const kson::ByPulse<kson::Interval>& lane, const kson::BeatInfo& beatInfo)
	{
		kson::ByPulse<JudgmentResult> judgmentArray;

		for (const auto& [y, note] : lane) // TODO: merge two long notes if note1.y + note1.l == note2.y
		{
			if (note.length > 0)
			{
				// Determine whether to halve the combo based on the BPM at the start of the note
				// (BPM changes during the notes are ignored)
				const bool halvesCombo = kson::TimingUtils::PulseTempo(y, beatInfo) >= kHalveComboBPMThreshold;
				const kson::RelPulse minPulseInterval = halvesCombo ? (kson::kResolution4 * 3 / 8) : (kson::kResolution4 * 3 / 16);
				const kson::RelPulse pulseInterval = halvesCombo ? (kson::kResolution4 / 8) : (kson::kResolution4 / 16);

				if (note.length <= minPulseInterval)
				{
					judgmentArray.emplace(y, JudgmentResult::kUnspecified);
				}
				else
				{
					const kson::Pulse start = ((y + pulseInterval - 1) / pulseInterval + 1) * pulseInterval;
					const kson::Pulse end = y + note.length - pulseInterval;

					for (kson::Pulse pulse = start; pulse < end; pulse += pulseInterval)
					{
						judgmentArray.emplace(pulse, JudgmentResult::kUnspecified);
					}
				}
			}
		}

		return judgmentArray;
	}
}

void ButtonLaneJudgment::processKeyDown(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse, double currentTimeSec, Graphics::LaneState& laneStateRef)
{
	using namespace TimingWindow;

	// Pick up the nearest note from the lane
	bool found = false;
	double minDistance;
	kson::Pulse nearestNotePulse;
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
				laneStateRef.currentLongNotePulse = y;
				laneStateRef.currentLongNoteAnimOffsetTimeSec = currentTimeSec;
				return;
			}
			else if (found && sec - currentTimeSec > LongNote::kWindowSecPreHold && y > currentPulse)
			{
				break;
			}
		}
	}

	laneStateRef.keyBeamTimeSec = currentTimeSec;
	laneStateRef.keyBeamType = KeyBeamType::kDefault;

	if (found)
	{
		Optional<JudgmentResult> chipAnimType = none;
		if (minDistance < ChipNote::kWindowSecCritical)
		{
			m_chipJudgmentArray.at(nearestNotePulse) = JudgmentResult::kCritical;
			m_scoreValue += kScoreValueCritical;
			laneStateRef.keyBeamType = KeyBeamType::kCritical;
			chipAnimType = JudgmentResult::kCritical;
		}
		else if (minDistance < ChipNote::kWindowSecNear)
		{
			m_chipJudgmentArray.at(nearestNotePulse) = JudgmentResult::kNear;
			m_scoreValue += kScoreValueNear;
			laneStateRef.keyBeamType = KeyBeamType::kNear; // TODO: fast/slow
			chipAnimType = JudgmentResult::kNear; // TODO: fast/slow
		}
		else if (minDistance < ChipNote::kWindowSecError) // TODO: easy gauge, fast/slow
		{
			m_chipJudgmentArray.at(nearestNotePulse) = JudgmentResult::kError;
			laneStateRef.keyBeamType = KeyBeamType::kDefault;
			chipAnimType = JudgmentResult::kError;
		}

		if (chipAnimType.has_value())
		{
			assert(0 <= laneStateRef.chipAnimStateRingBufferCursor && laneStateRef.chipAnimStateRingBufferCursor < Graphics::kChipAnimMax);
			laneStateRef.chipAnimStateRingBuffer[laneStateRef.chipAnimStateRingBufferCursor] = {
				.startTimeSec = currentTimeSec,
				.type = *chipAnimType,
			};
			laneStateRef.chipAnimStateRingBufferCursor = (laneStateRef.chipAnimStateRingBufferCursor + 1) % Graphics::kChipAnimMax;
		}
	}
}

void MusicGame::Judgment::ButtonLaneJudgment::processKeyPressed(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse, double currentSec, const Graphics::LaneState& laneStateRef)
{
	if (laneStateRef.currentLongNotePulse.has_value())
	{
		const kson::Pulse noteStartPulse = *laneStateRef.currentLongNotePulse;
		const kson::Pulse noteEndPulse = *laneStateRef.currentLongNotePulse + lane.at(*laneStateRef.currentLongNotePulse).length;
		const kson::Pulse limitPulse = Min(currentPulse, noteEndPulse);

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

ButtonLaneJudgment::ButtonLaneJudgment(KeyConfig::Button keyConfigButton, const kson::ByPulse<kson::Interval>& lane, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache)
	: m_keyConfigButton(keyConfigButton)
	, m_pulseToSec(CreatePulseToSec(lane, beatInfo, timingCache))
	, m_chipJudgmentArray(CreateChipNoteJudgmentArray(lane))
	, m_longJudgmentArray(CreateLongNoteJudgmentArray(lane, beatInfo))
	, m_scoreValueMax(static_cast<int32>(m_chipJudgmentArray.size() + m_longJudgmentArray.size()) * kScoreValueCritical)
{
}

void MusicGame::Judgment::ButtonLaneJudgment::update(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse, double currentTimeSec, Graphics::LaneState& laneStateRef)
{
	// Chip note & long note start
	if (KeyConfig::Down(m_keyConfigButton))
	{
		processKeyDown(lane, currentPulse, currentTimeSec, laneStateRef);
	}

	// Long note hold
	if (KeyConfig::Pressed(m_keyConfigButton))
	{
		processKeyPressed(lane, currentPulse, currentTimeSec, laneStateRef);
	}

	// Long note release
	if (laneStateRef.currentLongNotePulse.has_value() &&
		(KeyConfig::Up(m_keyConfigButton) || (*laneStateRef.currentLongNotePulse + lane.at(*laneStateRef.currentLongNotePulse).length < currentPulse)))
	{
		laneStateRef.currentLongNotePulse = none;
		laneStateRef.currentLongNoteAnimOffsetTimeSec = currentTimeSec;
	}

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
