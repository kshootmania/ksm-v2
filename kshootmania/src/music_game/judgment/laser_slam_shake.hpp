#pragma once
#include "music_game/camera/cam_system.hpp"

// TODO: Judgmentではないので適切な名前空間を作る
namespace MusicGame::Judgment
{
	/// @brief 直角LASER判定後の振動に関する状態
	struct LaserSlamShake
	{
	private:
		// 最後に直角LASERを判定した時間
		double m_startSec = kPastTimeSec;

		// 最後に判定した直角LASERの方向(-1:左, +1:右)
		int32 m_direction = 0;

		/// @brief 現在の横方向のずらし量を取得
		/// @param currentTimeSec 現在の時間(秒)
		/// @return 視点制御のshiftX換算の値
		double shiftX(double currentTimeSec) const;

	public:
		LaserSlamShake() = default;
		
		/// @brief 直角LASER判定時に呼ぶ関数
		/// @param prevTimeSec 前回フレームの時間(秒)
		/// @param direction 直角LASERの方向(-1:左, +1:右)
		/// @note 前回フレームの時間を使用しているのは、フレームレートが低い場合でも即座に振動開始させるため
		void onLaserSlamJudged(double prevTimeSec, int32 direction);

		/// @brief 現在の振動をCamStatusに適用
		/// @param camStatusRef 適用先のCamStatusへの参照
		/// @param currentTimeSec 現在の時間(秒)
		/// @note この関数ではCamStatusに値を相対的に反映するので、判定と関係ないカメラの値はあらかじめ設定しておくこと
		void applyToCamStatus(Camera::CamStatus& camStatusRef, double currentTimeSec) const;
	};
}
