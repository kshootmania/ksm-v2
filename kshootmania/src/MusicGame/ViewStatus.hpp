#pragma once
#include "Camera/CamSystem.hpp"

namespace MusicGame
{
	/// @brief ビューステータス
	/// @note 表示に関与する状態を入れる。ゲームプレイに関与するものはViewStatusではなくGameStatusへ入れる。
	struct ViewStatus
	{
		// HUDアニメーションの開始時刻を「まだ一度も発生していない」状態にするための番兵値
		static constexpr double kPastDisplayTimeSec = -100000000.0;

		double tiltRadians = 0.0;
		double tiltRadiansForBgLayer = 0.0;

		Camera::CamStatus camStatus;

		int32 score = 0;
		double gaugePercentage = 0.0;
		int32 gaugePercentageInt = 0; // 切り捨てで整数化したゲージパーセンテージ
		int32 displayCombo = 0;
		bool displayIsNoError = true;
		double comboMilestoneEffectStartTimeSec = kPastDisplayTimeSec; // 100コンボ毎のアニメーションの開始時刻
		int32 timingAdjustMs = 0;
	};
}
