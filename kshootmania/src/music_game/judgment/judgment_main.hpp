#pragma once
#include "kson/chart_data.hpp"
#include "music_game/play_option.hpp"
#include "button_lane_judgment.hpp"
#include "laser_lane_judgment.hpp"
#include "judgment_handler.hpp"

namespace MusicGame::Judgment
{
	/// @brief ノーツ判定のメイン
	class JudgmentMain
	{
	private:
		const PlayOption m_playOption;
		BTLaneJudgments m_btLaneJudgments;
		FXLaneJudgments m_fxLaneJudgments;
		LaserLaneJudgments m_laserLaneJudgments;

		JudgmentHandler m_judgmentHandler;

	public:
		explicit JudgmentMain(const kson::ChartData& chartData, const kson::TimingCache& timingCache, const PlayOption& playOption);

		void update(const kson::ChartData& chartData, GameStatus& gameStatusRef, ViewStatus& viewStatusRef);

		void lockForExit();

		/// @brief PlayResultを取得
		/// @return PlayResult
		PlayResult playResult() const;

		/// @brief 全てのノーツが判定済みかどうか
		/// @return 全てのノーツが判定済みならtrue
		bool isFinished() const;
	};
}
