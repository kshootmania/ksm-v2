#include "kson/audio/key_sound.hpp"

void kson::to_json(nlohmann::json& j, const KeySoundParams& params)
{
	j = nlohmann::json::object();

	if (params.volume != 1.0)
	{
		j["vol"] = params.volume;
	}
}

void kson::to_json(nlohmann::json& j, const KeySoundDef& def)
{
	if (!nlohmann::json(def.v).empty())
	{
		j["v"] = def.v;
	}
}

bool kson::KeySoundFXInfo::empty() const
{
	return def.empty() && chipEvent.empty();
}

void kson::to_json(nlohmann::json& j, const KeySoundFXInfo& fx)
{
	if (!fx.def.empty())
	{
		j["def"] = fx.def;
	}

	if (!fx.chipEvent.empty())
	{
		j["chip_event"] = fx.chipEvent;
	}
}

bool kson::KeySoundLaserInfo::empty() const
{
	return def.empty() && slamEvent.empty();
}

void kson::to_json(nlohmann::json& j, const KeySoundLaserInfo& laser)
{
	if (!laser.def.empty())
	{
		j["def"] = laser.def;
	}

	if (!laser.slamEvent.empty())
	{
		j["slam_event"] = laser.slamEvent;
	}
}

bool kson::KeySoundInfo::empty() const
{
	return fx.empty() && laser.empty();
}

void kson::to_json(nlohmann::json& j, const KeySoundInfo& keySound)
{
	if (!keySound.fx.empty())
	{
		j["fx"] = keySound.fx;
	}

	if (!keySound.laser.empty())
	{
		j["laser"] = keySound.laser;
	}
}
