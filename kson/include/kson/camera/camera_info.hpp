#pragma once
#include "kson/common/common.hpp"
#include "cam.hpp"
#include "tilt.hpp"

namespace kson
{
	struct CameraInfo
	{
		CamInfo cam;
		TiltInfo tilt;

		bool empty() const;
	};
}
