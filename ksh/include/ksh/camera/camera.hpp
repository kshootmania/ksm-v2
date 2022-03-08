#pragma once
#include "ksh/common/common.hpp"
#include "cam.hpp"
#include "tilt.hpp"

namespace ksh
{
	struct CameraRoot
	{
		CamRoot cams;
		TiltRoot tilts;
	};
}
