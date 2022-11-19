#pragma once
#include "music_game/game_defines.hpp"
#include "music_game/game_status.hpp"
#include "kson/chart_data.hpp"
#include "kson/util/timing_utils.hpp"

namespace MusicGame::Judgment
{
	class ButtonLaneJudgment
	{
	private:
		const KeyConfig::Button m_keyConfigButton;
		const std::map<kson::Pulse, double> m_pulseToSec;

		kson::ByPulse<JudgmentResult> m_chipJudgmentArray;
		kson::ByPulse<JudgmentResult> m_longJudgmentArray;

		kson::Pulse m_prevPulse = kPastPulse;

		kson::Pulse m_passedNotePulse = kPastPulse;

		int32 m_scoreValue = 0;

		const int32 m_scoreValueMax;

		void processKeyDown(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse, double currentTimeSec, ButtonLaneStatus& laneStatusRef);

		void processKeyPressed(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse, const ButtonLaneStatus& laneStatusRef);

	public:
		ButtonLaneJudgment(KeyConfig::Button keyConfigButton, const kson::ByPulse<kson::Interval>& lane, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache);

		void update(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse, double currentTimeSec, ButtonLaneStatus& laneStatusRef);

		int32 scoreValue() const;

		int32 scoreValueMax() const;
	};
}
