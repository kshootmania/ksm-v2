#pragma once
#include "camera/cam_system.hpp"

namespace MusicGame
{
	/// @brief 直角LASER判定後の振動に関する状態
	struct LaserSlamWiggleStatus
	{
	private:
		// 最後に直角LASERを判定した時間
		double m_startSec = kPastTimeSec;

		// 最後に判定した直角LASERの方向(-1:左, +1:右)
		int32 m_direction = 0;

	public:
		/// @brief 直角LASER判定時に呼ぶ関数
		/// @param prevTimeSec 前回フレームの時間(秒)
		/// @param direction 直角LASERの方向(-1:左, +1:右)
		/// @note 前回フレームの時間を使用しているのは、フレームレートが低い場合でも即座に振動開始させるため
		void onLaserSlamJudged(double prevTimeSec, int32 direction)
		{
			m_startSec = prevTimeSec;
			m_direction = direction;
		}

		/// @brief 現在の横方向のずらし量を取得
		/// @param currentTimeSec 現在の時間(秒)
		/// @return 視点制御のshiftX換算の値
		double shiftX(double currentTimeSec) const
		{
			constexpr double kWiggleDurationSec = 0.15;
			constexpr double kWiggleMaxShiftX = 12.5;
			const double diffTimeSec = currentTimeSec - m_startSec;
			if (diffTimeSec < kWiggleDurationSec)
			{
				const double rate = diffTimeSec / kWiggleDurationSec;
				const double invRate = 1.0 - rate;
				return Sin(Math::Pi * rate) * invRate * invRate * m_direction * kWiggleMaxShiftX;
			}
			else
			{
				return 0.0;
			}
		}
	};

	/// @brief ビューステータス
	/// @note 表示に関与する状態を入れる。ゲームプレイに関与するものはViewStatusではなくGameStatusへ入れる。
	struct ViewStatus
	{
		double tiltRadians = 0.0;

		Camera::CamStatus camStatus;

		LaserSlamWiggleStatus laserSlamWiggleStatus;
	};
}
