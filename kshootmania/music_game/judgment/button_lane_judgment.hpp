#pragma once
#include "judgment_defines.hpp"
#include "music_game/game_defines.hpp"
#include "music_game/graphics/update_info.hpp"
#include "ksh/chart_data.hpp"
#include "ksh/util/timing_utils.hpp"

namespace MusicGame::Judgment
{
	class ButtonLaneJudgment
	{
	private:
		const KeyConfig::Button m_keyConfigButton;
		const std::map<ksh::Pulse, double> m_pulseToSec;

		ksh::ByPulse<JudgmentResult> m_chipJudgmentArray;
		ksh::ByPulse<JudgmentResult> m_longJudgmentArray;

		ksh::Pulse m_prevPulse = kPastPulse;

		ksh::Pulse m_passedNotePulse = kPastPulse;

		Optional<ksh::Pulse> m_currentLongNotePulse = none;

		int32 m_scoreValue = 0;

		const int32 m_scoreValueMax;

		Optional<KeyBeamType> processKeyDown(const ksh::ByPulse<ksh::Interval>& lane, ksh::Pulse currentPulse, double currentSec);

		void processKeyPressed(const ksh::ByPulse<ksh::Interval>& lane, ksh::Pulse currentPulse, double currentSec);

	public:
		ButtonLaneJudgment(KeyConfig::Button keyConfigButton, const ksh::ByPulse<ksh::Interval>& lane, const ksh::BeatMap& beatMap, const ksh::TimingCache& timingCache);

		void update(const ksh::ByPulse<ksh::Interval>& lane, ksh::Pulse currentPulse, double currentSec, Graphics::LaneState& laneStateRef);

		int32 scoreValue() const;

		int32 scoreValueMax() const;
	};
}
