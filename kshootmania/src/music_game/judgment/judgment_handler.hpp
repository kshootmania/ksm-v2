#pragma once
#include "kson/chart_data.hpp"
#include "music_game/view_status.hpp"
#include "judgment_defines.hpp"
#include "scoring_status.hpp"
#include "laser_slam_shake.hpp"

namespace MusicGame::Judgment
{
	class JudgmentHandler
	{
	private:
		ScoringStatus m_scoringStatus;

		LaserSlamShake m_laserSlamShakeStatus;

	public:
		/// @brief コンストラクタ
		/// @param chartData 譜面データ
		/// @param btLaneJudgments BTレーン判定の配列
		/// @param fxLaneJudgments FXレーン判定の配列
		/// @param laserLaneJudgments LASERレーン判定の配列
		JudgmentHandler(const kson::ChartData& chartData, const BTLaneJudgments& btLaneJudgments, const FXLaneJudgments& fxLaneJudgments, const LaserLaneJudgments& laserLaneJudgments);

		/// @brief チップノーツ判定時に呼び出される
		/// @param result 判定結果
		void onChipJudged(JudgmentResult result);

		/// @brief ロングノーツ判定時に呼び出される
		/// @param result 判定結果
		void onLongJudged(JudgmentResult result);

		/// @brief LASERのライン判定時に呼び出される
		/// @param result 判定結果
		void onLaserLineJudged(JudgmentResult result);

		/// @brief 直角LASER判定時に呼び出される
		/// @param result 判定結果
		/// @param prevTimeSec 前回フレームの時間(秒)
		/// @param direction 直角LASERの方向(-1:左, +1:右)
		void onLaserSlamJudged(JudgmentResult result, double prevTimeSec, int32 direction);

		/// @brief 状態をViewStatusに適用
		/// @param viewStatusRef 適用先のViewStatusへの参照
		/// @param currentTimeSec 現在の時間(秒)
		/// @note この関数ではViewStatus::camStatusに値を相対的に反映するので、判定と関係ないカメラの値はあらかじめ設定しておくこと
		void applyToViewStatus(ViewStatus& viewStatusRef, double currentTimeSec);
	};
}
