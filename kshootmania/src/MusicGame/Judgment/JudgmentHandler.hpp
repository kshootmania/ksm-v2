#pragma once
#include "kson/ChartData.hpp"
#include "MusicGame/PlayOption.hpp"
#include "MusicGame/PlayResult.hpp"
#include "MusicGame/ViewStatus.hpp"
#include "JudgmentDefines.hpp"
#include "ScoringStatus.hpp"
#include "LaserSlamShake.hpp"
#include "MusicGame/Camera/CamPattern/CamPatternMain.hpp"

namespace MusicGame::Judgment
{
	class JudgmentHandler
	{
	private:
		const PlayOption m_playOption;

		const int32 m_totalCombo;

		bool m_isLockedForExit = false;

		ScoringStatus m_scoringStatus;

		LaserSlamShake m_laserSlamShakeStatus;

		Camera::CamPatternMain m_camPatternMain;

		// タイミング調整オフセット(秒)
		double m_timingAdjustOffsetSec = 0.0;

		// 前回ViewStatus反映時のコンボ数(100コンボ毎のアニメーションの検知に使用)
		int32 m_prevDisplayCombo = 0;

		// 100コンボ毎のアニメーションの開始時刻(秒)
		double m_comboMilestoneEffectStartTimeSec = ViewStatus::kPastDisplayTimeSec;

	public:
		/// @brief コンストラクタ
		/// @param chartData 譜面データ
		/// @param btLaneJudgments BTレーン判定の配列
		/// @param fxLaneJudgments FXレーン判定の配列
		/// @param laserLaneJudgments LASERレーン判定の配列
		/// @param playOption プレイオプション
		/// @param courseContinuation コース継続情報(コースモード時に使用)
		/// @param gameMode ゲームモード
		JudgmentHandler(const kson::ChartData& chartData, const BTLaneJudgments& btLaneJudgments, const FXLaneJudgments& fxLaneJudgments, const LaserLaneJudgments& laserLaneJudgments, const PlayOption& playOption, const Optional<CourseContinuation>& courseContinuation, GameMode gameMode);

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
		/// @param laserSlamPulse 直角LASERのPulse値
		/// @param prevTimeSec 前回フレームの時間(秒)
		/// @param prevPulse 前回フレームのPulse値
		/// @param direction 直角LASERの方向(-1:左, +1:右)
		void onLaserSlamJudged(JudgmentResult result, kson::Pulse laserSlamPulse, double prevTimeSec, kson::Pulse prevPulse, int32 direction);

		/// @brief 状態をViewStatusに適用
		/// @param viewStatusRef 適用先のViewStatusへの参照
		/// @param currentTimeSec 現在の時間(秒)
		/// @param currentPulse 現在のPulse値
		/// @note この関数ではViewStatus::camStatusに値を相対的に反映するので、判定と関係ないカメラの値はあらかじめ設定しておくこと
		void applyToViewStatus(ViewStatus& viewStatusRef, double currentTimeSec, kson::Pulse currentPulse);

		/// @brief プレイ終了のために判定処理をロック
		void lockForExit();

		/// @brief 全てのコンボを判定済みかどうかを取得
		bool isFinished() const;

		/// @brief PlayResultを取得
		/// @param currentTimeSec 現在時刻(秒)
		/// @param chartEndTimeSec 譜面終了時刻(秒)
		/// @param isHardFailed HARDゲージ/コースモードで途中落ちしたかどうか
		/// @param isAborted Backボタンで途中終了したかどうか
		/// @return PlayResult
		PlayResult playResult(double currentTimeSec, double chartEndTimeSec, IsHardFailedYN isHardFailed, bool isAborted) const;

		/// @brief タイミング調整オフセットを取得(秒)
		[[nodiscard]]
		double timingAdjustOffsetSec() const;

		/// @brief AutoSyncによるタイミング調整を適用
		/// @param diffSec 判定タイミング差分(秒単位、FAST側が正、SLOW側が負)
		void applyAutoSyncTimingAdjust(double diffSec);

		/// @brief タイミング調整オフセットを加算(秒)
		void addTimingAdjustOffset(double offsetSec);
	};
}
