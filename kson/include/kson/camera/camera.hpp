#pragma once
#include "kson/common/common.hpp"
#include "cam.hpp"
#include "tilt.hpp"

namespace kson
{
	struct CameraRoot
	{
		CamRoot cam;
		TiltRoot tilt;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const CameraRoot& camera);
}
