#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct KeySoundParams
	{
		double volume = 1.0;
	};

	inline void to_json(nlohmann::json& j, const KeySoundParams& params)
	{
		j = nlohmann::json::object();

		if (params.volume != 1.0)
		{
			j["vol"] = params.volume;
		}
	}

	struct KeySoundDef
	{
		std::string filename; // UTF-8 guaranteed

		KeySoundParams defaultParams;
	};

	inline void to_json(nlohmann::json& j, const KeySoundDef& def)
	{
		j["filename"] = def.filename;

		if (!nlohmann::json(def.defaultParams).empty())
		{
			j["v"] = def.defaultParams;
		}
	}

	struct KeySoundRoot
	{
		DefList<KeySoundDef> defList;
		InvokeList<ByPulse<KeySoundParams>> pulseEventList;
		InvokeList<ByNotes<KeySoundParams>> noteEventList;

		bool empty() const
		{
			return defList.empty() && pulseEventList.empty() && noteEventList.empty();
		}
	};

	inline void to_json(nlohmann::json& j, const KeySoundRoot& keySound)
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
}
