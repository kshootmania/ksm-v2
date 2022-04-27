#pragma once
#include "judgment_defines.hpp"
#include "music_game/game_defines.hpp"
#include "ksh/chart_data.hpp"
#include "ksh/util/timing_utils.hpp"

namespace MusicGame::Judgment
{
	class ButtonLaneJudgment
	{
	private:
		const std::map<ksh::Pulse, double> m_pulseToSec;

		ksh::ByPulse<JudgmentResult> m_chipJudgmentArray;
		ksh::ByPulse<JudgmentResult> m_longJudgmentArray;

		ksh::Pulse m_passedNotePulse = kPastPulse;

		Optional<ksh::Pulse> m_currentHoldingLongNotePulse = none;

	public:
		ButtonLaneJudgment(const ksh::ByPulse<ksh::Interval>& lane, const ksh::BeatMap& beatMap, const ksh::TimingCache& timingCache);

		Optional<KeyBeamType> processKeyDown(const ksh::ByPulse<ksh::Interval>& lane, ksh::Pulse currentPulse, double currentSec);
	};
}
