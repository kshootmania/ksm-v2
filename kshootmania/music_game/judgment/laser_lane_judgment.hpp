#pragma once
#include "music_game/game_defines.hpp"
#include "music_game/game_status.hpp"
#include "kson/chart_data.hpp"
#include "kson/util/timing_utils.hpp"

namespace MusicGame::Judgment
{
	class LaserLaneJudgment
	{
	private:
		const KeyConfig::Button m_keyConfigButtonL;
		const KeyConfig::Button m_keyConfigButtonR;
		const std::map<kson::Pulse, double> m_pulseToSec;
		const kson::ByPulse<int32> m_laserLineDirectionMap;

		kson::ByPulse<JudgmentResult> m_lineJudgmentArray;
		kson::ByPulse<JudgmentResult> m_slamJudgmentArray;

		Optional<kson::Pulse> m_prevCurrentLaserSectionPulse = none;

		int32 m_scoreValue = 0;

		const int32 m_scoreValueMax;

		void processKeyPressed(KeyConfig::Button button, const kson::ByPulse<kson::LaserSection>& lane, kson::Pulse currentPulse, LaserLaneStatus& laneStatusRef);

	public:
		LaserLaneJudgment(KeyConfig::Button keyConfigButtonL, KeyConfig::Button keyConfigButtonR, const kson::ByPulse<kson::LaserSection>& lane, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache);

		void update(const kson::ByPulse<kson::LaserSection>& lane, kson::Pulse currentPulse, double currentSec, LaserLaneStatus& laneStatusRef);

		int32 scoreValue() const;

		int32 scoreValueMax() const;
	};
}
