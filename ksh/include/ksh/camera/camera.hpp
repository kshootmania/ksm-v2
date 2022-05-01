#pragma once
#include "ksh/common/common.hpp"
#include "cam.hpp"
#include "tilt.hpp"

namespace ksh
{
	struct CameraRoot
	{
		CamRoot cam;
		TiltRoot tilt;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const CameraRoot& camera);
}
