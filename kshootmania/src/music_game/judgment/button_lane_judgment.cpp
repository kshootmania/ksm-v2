﻿#include "button_lane_judgment.hpp"
#include "music_game/graphics/graphics_defines.hpp"

namespace MusicGame::Judgment
{
	namespace
	{
		uint64 eventIndex = 0;
		std::map<kson::Pulse, double> CreatePulseToSec(const kson::ByPulse<kson::Interval>& lane, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache)
		{
			std::map<kson::Pulse, double> pulseToSec;

			for (const auto& [y, note] : lane)
			{
				if (!pulseToSec.contains(y))
				{
					const double sec = kson::PulseToSec(y, beatInfo, timingCache);
					pulseToSec.emplace(y, sec);
				}
				if (!pulseToSec.contains(y + note.length))
				{
					const double sec = kson::PulseToSec(y + note.length, beatInfo, timingCache);
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
					// BPMをもとにロングノーツのコンボ数を半減させるかを決める
					// (ノーツ途中でのBPM変更は特に加味しない)
					const bool halvesCombo = kson::TempoAt(y, beatInfo) >= kHalveComboBPMThreshold;
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

		bool IsDuringLongNote(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse)
		{
			const auto& currentNoteItr = kson::ValueItrAt(lane, currentPulse);
			if (currentNoteItr != lane.end())
			{
				const auto& [y, note] = *currentNoteItr;
				if (y <= currentPulse && currentPulse < y + note.length)
				{
					return true;
				}
			}
			return false;
		}
	}

	void ButtonLaneJudgment::processKeyDown(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse, double currentTimeSec, ButtonLaneStatus& laneStatusRef)
	{
		using namespace TimingWindow;

		// レーン上で最も現在時間に近いノーツを調べる
		bool found = false;
		double minDistance = 0.0;
		double minDistanceWithSgn = 0.0;
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
					minDistanceWithSgn = sec - currentTimeSec;
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
					laneStatusRef.currentLongNotePulse = y;
					laneStatusRef.currentLongNoteAnimOffsetTimeSec = currentTimeSec;
					return;
				}
				else if (found && sec - currentTimeSec > LongNote::kWindowSecPreHold && y > currentPulse)
				{
					break;
				}
			}
		}

		laneStatusRef.keyBeamTimeSec = currentTimeSec;
		laneStatusRef.keyBeamType = KeyBeamType::kDefault;

		if (found)
		{
			Optional<JudgmentResult> chipAnimType = none;
			if (minDistance < ChipNote::kWindowSecCritical)
			{
				m_chipJudgmentArray.at(nearestNotePulse) = JudgmentResult::kCritical;
				m_scoreValue += kScoreValueCritical;
				laneStatusRef.keyBeamType = KeyBeamType::kCritical;
				chipAnimType = JudgmentResult::kCritical;
				Print << U"CRIT:" << MathUtils::RoundToInt(minDistanceWithSgn * 1000);
				std::cout << MathUtils::RoundToInt(minDistanceWithSgn * 1000) << '\n';
			}
			else if (minDistance < ChipNote::kWindowSecNear)
			{
				m_chipJudgmentArray.at(nearestNotePulse) = JudgmentResult::kNear;
				m_scoreValue += kScoreValueNear;
				laneStatusRef.keyBeamType = KeyBeamType::kNear; // TODO: fast/slow
				chipAnimType = JudgmentResult::kNear; // TODO: fast/slow
				Print << U"NEAR:" << MathUtils::RoundToInt(minDistanceWithSgn * 1000);
			}
			else if (minDistance < ChipNote::kWindowSecError) // TODO: easy gauge, fast/slow
			{
				m_chipJudgmentArray.at(nearestNotePulse) = JudgmentResult::kError;
				laneStatusRef.keyBeamType = KeyBeamType::kDefault;
				chipAnimType = JudgmentResult::kError;
			}

			if (chipAnimType.has_value())
			{
				assert(laneStatusRef.chipAnimStateRingBufferCursor < Graphics::kChipAnimMaxPlaying);
				laneStatusRef.chipAnimStatusRingBuffer[laneStatusRef.chipAnimStateRingBufferCursor] = {
					.startTimeSec = currentTimeSec,
					.type = *chipAnimType,
				};
				laneStatusRef.chipAnimStateRingBufferCursor = (laneStatusRef.chipAnimStateRingBufferCursor + 1U) % Graphics::kChipAnimMaxPlaying;
			}
		}
	}

	void ButtonLaneJudgment::processKeyPressed(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse, const ButtonLaneStatus& laneStatusRef)
	{
		if (laneStatusRef.currentLongNotePulse.has_value())
		{
			const kson::Pulse noteStartPulse = laneStatusRef.currentLongNotePulse.value();
			const kson::Pulse noteEndPulse = noteStartPulse + lane.at(noteStartPulse).length;
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

	void ButtonLaneJudgment::update(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse, double currentTimeSec, ButtonLaneStatus& laneStatusRef)
	{
		// チップノーツとロングノーツの始点の判定処理
		int32 diffMilliseconds;
		if (KeyConfig::Down(m_keyConfigButton, &diffMilliseconds))
		{
			Print << diffMilliseconds;
			processKeyDown(lane, currentPulse, currentTimeSec - diffMilliseconds / 1000.0, laneStatusRef);
		}

		// ロングノーツ押下中の判定処理
		if (KeyConfig::Pressed(m_keyConfigButton))
		{
			processKeyPressed(lane, currentPulse, laneStatusRef);
		}

		// ロングノーツを離したときの判定処理
		if (laneStatusRef.currentLongNotePulse.has_value() &&
			(KeyConfig::Up(m_keyConfigButton) || (*laneStatusRef.currentLongNotePulse + lane.at(*laneStatusRef.currentLongNotePulse).length < currentPulse)))
		{
			laneStatusRef.currentLongNotePulse = none;
			laneStatusRef.currentLongNoteAnimOffsetTimeSec = currentTimeSec;
		}

		if (IsDuringLongNote(lane, currentPulse))
		{
			laneStatusRef.longNotePressed = laneStatusRef.currentLongNotePulse.has_value();
		}
		else
		{
			laneStatusRef.longNotePressed = none;
		}

		m_prevPulse = currentPulse;
	}

	int32 ButtonLaneJudgment::scoreValue() const
	{
		return m_scoreValue;
	}

	int32 ButtonLaneJudgment::scoreValueMax() const
	{
		return m_scoreValueMax;
	}
}
