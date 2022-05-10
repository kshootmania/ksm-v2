#include "kson/camera/cam.hpp"

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

	if (params.repeatScale != 1.0)
	{
		j["repeat_scale"] = params.repeatScale;
	}

	if (params.decayOrder != 0.0)
	{
		j["decay_order"] = params.decayOrder;
	}
}

void kson::to_json(nlohmann::json& j, const CamPatternDef& def)
{
	j = {
		{ "body", def.body },
		{ "v", def.params },
	};
}

bool kson::CamPatternLaserInfo::empty() const
{
	return def.empty() && slamInvoke.empty();
}

void kson::to_json(nlohmann::json& j, const CamPatternLaserInfo& info)
{
	j = nlohmann::json::object();

	if (!info.def.empty())
	{
		j["def"] = info.def;
	}

	if (!info.slamInvoke.empty())
	{
		j["slam_invoke"] = info.slamInvoke;
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
