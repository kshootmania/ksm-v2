#include "kson/camera/camera_info.hpp"

bool kson::CameraInfo::empty() const
{
	return cam.empty() && tilt.empty();
}

void kson::to_json(nlohmann::json& j, const CameraInfo& camera)
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
