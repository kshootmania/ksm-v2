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

		bool empty() const
		{
			return cam.empty() && tilt.empty();
		}
	};

	inline void to_json(nlohmann::json& j, const CameraRoot& camera)
	{
		j = nlohmann::json::object();

		if (!camera.cam.empty())
		{
			j["cam"] = camera.cam;
		}

		if (!camera.tilt.empty())
		{
			j["tilt"] = camera.tilt;
		}
	}
}
