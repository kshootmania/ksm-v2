#include "kson/camera/camera.hpp"

bool kson::CameraRoot::empty() const
{
	return cam.empty() && tilt.empty();
}

void kson::to_json(nlohmann::json& j, const CameraRoot& camera)
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
