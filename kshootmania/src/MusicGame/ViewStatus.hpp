#pragma once
#include "Camera/CamSystem.hpp"

namespace MusicGame
{
	/// @brief ビューステータス
	/// @note 表示に関与する状態を入れる。ゲームプレイに関与するものはViewStatusではなくGameStatusへ入れる。
	struct ViewStatus
	{
		double tiltRadians = 0.0;
		double tiltRadiansForBgLayer = 0.0;

		Camera::CamStatus camStatus;

		int32 score = 0;
		double gaugePercentage = 0.0;
		int32 gaugePercentageInt = 0; // 切り捨てで整数化したゲージパーセンテージ
		int32 displayCombo = 0;
		bool displayIsNoError = true;
	};
}
