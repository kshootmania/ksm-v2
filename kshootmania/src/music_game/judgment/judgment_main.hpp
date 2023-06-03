#pragma once
#include "kson/chart_data.hpp"
#include "button_lane_judgment.hpp"
#include "laser_lane_judgment.hpp"
#include "judgment_handler.hpp"

namespace MusicGame::Judgment
{
	/// @brief ノーツ判定のメイン
	class JudgmentMain
	{
	private:
		BTLaneJudgments m_btLaneJudgments;
		FXLaneJudgments m_fxLaneJudgments;
		LaserLaneJudgments m_laserLaneJudgments;

		JudgmentHandler m_judgmentHandler;

	public:
		explicit JudgmentMain(const kson::ChartData& chartData, const kson::TimingCache& timingCache);

		void update(const kson::ChartData& chartData, GameStatus& gameStatusRef, ViewStatus& viewStatusRef);
	};
}
