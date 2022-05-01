#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct KeySoundParams
	{
		double volume = 1.0;
	};

	void to_json(nlohmann::json& j, const KeySoundParams& params);

	struct KeySoundDef
	{
		std::string filename; // UTF-8 guaranteed

		KeySoundParams defaultParams;
	};

	void to_json(nlohmann::json& j, const KeySoundDef& def);

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

	void to_json(nlohmann::json& j, const KeySoundRoot& keySound);
}
