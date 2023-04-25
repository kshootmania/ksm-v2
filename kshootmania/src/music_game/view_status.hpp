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
	};
}
