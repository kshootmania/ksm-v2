#pragma once
#include "kson/chart_data.hpp"
#include "button_lane_judgment.hpp"
#include "laser_lane_judgment.hpp"
#include "scoring_status.hpp"

namespace MusicGame::Judgment
{
	/// @brief ノーツ判定のメイン
	class JudgmentMain
	{
	private:
		std::array<Judgment::ButtonLaneJudgment, kson::kNumBTLanesSZ> m_btLaneJudgments;
		std::array<Judgment::ButtonLaneJudgment, kson::kNumFXLanesSZ> m_fxLaneJudgments;
		std::array<Judgment::LaserLaneJudgment, kson::kNumLaserLanesSZ> m_laserLaneJudgments;

		ScoringStatus m_scoringStatus;

	public:
		explicit JudgmentMain(const kson::ChartData& chartData, const kson::TimingCache& timingCache);

		void update(const kson::ChartData& chartData, GameStatus& gameStatusRef, ViewStatus& viewStatusRef);
	};
}
