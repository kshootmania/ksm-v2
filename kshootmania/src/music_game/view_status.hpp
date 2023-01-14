#pragma once
#include "scroll/hispeed_setting.hpp"

namespace MusicGame
{
	struct CamStatus
	{
		double zoom = 0.0;
		double shiftX = 0.0;
		double rotationX = 0.0;
		double rotationZ = 0.0;
		double rotationZLane = 0.0;
		double rotationZJdgLine = 0.0;
	};

	/// @brief ビューステータス
	/// @note 表示に関与する状態を入れる。ゲームプレイに関与するものはViewStatusではなくGameStatusへ入れる。
	struct ViewStatus
	{
		double tiltRadians = 0.0;

		CamStatus camStatus;
	};
}
