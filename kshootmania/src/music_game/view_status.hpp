#pragma once
#include "camera/cam_system.hpp"

namespace MusicGame
{
	/// @brief ビューステータス
	/// @note 表示に関与する状態を入れる。ゲームプレイに関与するものはViewStatusではなくGameStatusへ入れる。
	struct ViewStatus
	{
		double tiltRadians = 0.0;

		Camera::CamStatus camStatus;

		int32 score = 0;
		double gaugePercentage = 0.0;
		int32 combo = 0;
		bool isNoError = true;
	};
}
