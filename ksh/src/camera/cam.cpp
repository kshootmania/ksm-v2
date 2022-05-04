#include "ksh/camera/cam.hpp"

void ksh::to_json(nlohmann::json& j, const CamPatternParams& params)
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

void ksh::to_json(nlohmann::json& j, const CamPatternDef& def)
{
	j = {
		{ "body", def.body },
		{ "v", def.params },
	};
}

bool ksh::CamPatternInfo::empty() const
{
	return defList.empty() && pulseEventList.empty() && noteEventList.empty();
}

void ksh::to_json(nlohmann::json& j, const CamPatternInfo& info)
{
	j = nlohmann::json::object();

	if (!info.defList.empty())
	{
		j["def"] = info.defList;
	}

	if (!info.pulseEventList.empty())
	{
		j["pulse_event"] = info.pulseEventList;
	}

	if (!info.noteEventList.empty())
	{
		j["note_event"] = info.noteEventList;
	}
}

bool ksh::CamRoot::empty() const
{
	return body.empty() && patternInfo.empty();
}

void ksh::to_json(nlohmann::json& j, const CamRoot& cam)
{
	j = nlohmann::json::object();

	if (!cam.body.empty())
	{
		j["body"] = cam.body;
	}

	if (!cam.patternInfo.empty())
	{
		j["pattern"] = cam.patternInfo;
	}
}
