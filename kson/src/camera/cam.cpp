#include "kson/camera/cam.hpp"

bool kson::CamGraphs::empty() const
{
	return
		zoom.empty() &&
		shiftX.empty() &&
		rotationX.empty() &&
		rotationZ.empty() &&
		rotationZLane.empty() &&
		rotationZJdgLine.empty() &&
		centerSplit.empty();
}

void kson::to_json(nlohmann::json& j, const CamGraphs& camGraphs)
{
	j = nlohmann::json::object();

	if (!camGraphs.zoom.empty())
	{
		j["zoom"] = camGraphs.zoom;
	}

	if (!camGraphs.shiftX.empty())
	{
		j["shift_x"] = camGraphs.shiftX;
	}

	if (!camGraphs.rotationX.empty())
	{
		j["rotation_x"] = camGraphs.rotationX;
	}

	if (!camGraphs.rotationZ.empty())
	{
		j["rotation_z"] = camGraphs.rotationZ;
	}

	if (!camGraphs.rotationZLane.empty())
	{
		j["rotation_z.lane"] = camGraphs.rotationZLane;
	}

	if (!camGraphs.rotationZJdgLine.empty())
	{
		j["rotation_z.jdgline"] = camGraphs.rotationZJdgLine;
	}

	if (!camGraphs.centerSplit.empty())
	{
		j["center_split"] = camGraphs.centerSplit;
	}
}

void kson::to_json(nlohmann::json& j, const CamPatternParams& params)
{
	j["l"] = params.length;

	if (params.scale != 1.0)
	{
		j["scale"] = params.scale;
	}

	if (params.repeat != 1)
	{
		j["repeat"] = params.repeat;
	}

	if (params.decayOrder != 0.0)
	{
		j["decay_order"] = params.decayOrder;
	}
}

bool kson::CamPatternLaserInfo::empty() const
{
	return slamEvent.empty();
}

void kson::to_json(nlohmann::json& j, const CamPatternLaserInfo& info)
{
	j = nlohmann::json::object();

	if (!info.slamEvent.empty())
	{
		j["slam_event"] = info.slamEvent;
	}
}

bool kson::CamPatternInfo::empty() const
{
	return laser.empty();
}

void kson::to_json(nlohmann::json& j, const CamPatternInfo& info)
{
	j = {
		{ "laser", info.laser },
	};
}

bool kson::CamInfo::empty() const
{
	return body.empty() && pattern.empty();
}

void kson::to_json(nlohmann::json& j, const CamInfo& cam)
{
	j = nlohmann::json::object();

	if (!cam.body.empty())
	{
		j["body"] = cam.body;
	}

	if (!cam.pattern.empty())
	{
		j["pattern"] = cam.pattern;
	}
}
