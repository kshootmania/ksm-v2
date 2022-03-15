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
		std::u8string filename;

		KeySoundParams defaultParams;
	};

	inline void to_json(nlohmann::json& j, const KeySoundDef& def)
	{
		j["filename"] = UnU8(def.filename);

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
			j["pulse_event"] = UnU8(keySound.pulseEventList);
		}

		if (!keySound.noteEventList.empty())
		{
			j["note_event"] = UnU8(keySound.noteEventList);
		}
	}
}
