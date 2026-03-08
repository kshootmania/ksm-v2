#pragma once
#include "kson/ChartData.hpp"
#include "MusicGame/PlayOption.hpp"
#include "ButtonLaneJudgment.hpp"
#include "LaserLaneJudgment.hpp"
#include "JudgmentHandler.hpp"

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
		explicit JudgmentMain(const kson::ChartData& chartData, const kson::TimingCache& timingCache, const PlayOption& playOption, const Optional<CourseContinuation>& courseContinuation, GameMode gameMode);

		void update(const kson::ChartData& chartData, GameStatus& gameStatusRef, ViewStatus& viewStatusRef);

		/// @brief プレイ終了のために判定処理をロックし、残りの未判定ノーツをERROR判定にする
		void lockForExit();

		/// @brief PlayResultを取得
		/// @param chartData 譜面データ
		/// @param timingCache タイミングキャッシュ
		/// @param currentTimeSec 現在時刻(秒)
		/// @param isHardFailed HARDゲージ/コースモードで途中落ちしたかどうか
		/// @param isAborted Backボタンで途中終了したかどうか
		/// @return PlayResult
		PlayResult playResult(const kson::ChartData& chartData, const kson::TimingCache& timingCache, double currentTimeSec, IsHardFailedYN isHardFailed, bool isAborted) const;

		/// @brief 全てのノーツが判定済みかどうか
		/// @return 全てのノーツが判定済みならtrue
		bool isFinished() const;
	};
}
