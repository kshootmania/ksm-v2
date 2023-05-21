#pragma once
#include "music_game/game_defines.hpp"
#include "music_game/game_status.hpp"
#include "kson/chart_data.hpp"
#include "kson/util/timing_utils.hpp"

namespace MusicGame::Judgment
{
	class ButtonLaneJudgment
	{
	public:
		struct LongNoteJudgment
		{
			kson::RelPulse length = kson::RelPulse{ 0 };

			JudgmentResult result = JudgmentResult::kUnspecified;
		};

	private:
		const KeyConfig::Button m_keyConfigButton;
		const std::map<kson::Pulse, double> m_pulseToSec;

		kson::ByPulse<JudgmentResult> m_chipJudgmentArray;
		kson::ByPulse<LongNoteJudgment> m_longJudgmentArray;

		kson::Pulse m_prevPulse = kPastPulse;

		kson::ByPulse<kson::Interval>::const_iterator m_passedNoteCursor;
		kson::ByPulse<LongNoteJudgment>::iterator m_passedLongJudgmentCursor;

		void processKeyDown(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse, double currentTimeSec, ButtonLaneStatus& laneStatusRef, ScoringStatus& scoringStatusRef);

		void processKeyPressed(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse, const ButtonLaneStatus& laneStatusRef, ScoringStatus& scoringStatusRef);

		void processPassedNoteJudgment(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse, double currentTimeSec, ButtonLaneStatus& laneStatusRef, ScoringStatus& scoringStatusRef);

	public:
		ButtonLaneJudgment(KeyConfig::Button keyConfigButton, const kson::ByPulse<kson::Interval>& lane, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache);

		void update(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse, double currentTimeSec, ButtonLaneStatus& laneStatusRef, ScoringStatus& scoringStatusRef);

		std::size_t chipJudgmentCount() const;

		std::size_t longJudgmentCount() const;
	};
}
