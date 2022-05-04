#include "ksh/audio/key_sound.hpp"

void ksh::to_json(nlohmann::json& j, const KeySoundParams& params)
{
	j = nlohmann::json::object();

	if (params.volume != 1.0)
	{
		j["vol"] = params.volume;
	}
}

void ksh::to_json(nlohmann::json& j, const KeySoundDef& def)
{
	if (!nlohmann::json(def.v).empty())
	{
		j["v"] = def.v;
	}
}

bool ksh::KeySoundFXInfo::empty() const
{
	return def.empty() && chipInvoke.empty();
}

void ksh::to_json(nlohmann::json& j, const KeySoundFXInfo& fx)
{
	if (!fx.def.empty())
	{
		j["def"] = fx.def;
	}

	if (!fx.chipInvoke.empty())
	{
		j["chip_invoke"] = fx.chipInvoke;
	}
}

bool ksh::KeySoundLaserInfo::empty() const
{
	return def.empty() && slamInvoke.empty();
}

void ksh::to_json(nlohmann::json& j, const KeySoundLaserInfo& laser)
{
	if (!laser.def.empty())
	{
		j["def"] = laser.def;
	}

	if (!laser.slamInvoke.empty())
	{
		j["slam_invoke"] = laser.slamInvoke;
	}
}

bool ksh::KeySoundRoot::empty() const
{
	return fx.empty() && laser.empty();
}

void ksh::to_json(nlohmann::json& j, const KeySoundRoot& keySound)
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
