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
	j["filename"] = def.filename;

	if (!nlohmann::json(def.defaultParams).empty())
	{
		j["v"] = def.defaultParams;
	}
}

void ksh::to_json(nlohmann::json& j, const KeySoundRoot& keySound)
{
	if (!keySound.defList.empty())
	{
		j["def"] = keySound.defList;
	}

	if (!keySound.pulseEventList.empty())
	{
		j["pulse_event"] = keySound.pulseEventList;
	}

	if (!keySound.noteEventList.empty())
	{
		j["note_event"] = keySound.noteEventList;
	}
}
