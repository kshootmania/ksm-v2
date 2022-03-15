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

		bool empty() const
		{
			return cams.empty() && tilts.empty();
		}
	};

	inline void to_json(nlohmann::json& j, const CameraRoot& camera)
	{
		j = nlohmann::json::object();

		if (!camera.cams.empty())
		{
			j["cam"] = camera.cams;
		}

		if (!camera.tilts.empty())
		{
			j["tilt"] = camera.tilts;
		}
	}
}
