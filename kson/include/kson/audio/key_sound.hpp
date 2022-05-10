#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct KeySoundParams
	{
		double volume = 1.0;
	};

	void to_json(nlohmann::json& j, const KeySoundParams& params);

	struct KeySoundDef
	{
		KeySoundParams v;
	};

	void to_json(nlohmann::json& j, const KeySoundDef& def);

	struct KeySoundFXInfo
	{
		DefList<KeySoundDef> def;
		InvokeList<std::array<ByPulse<KeySoundParams>, kNumFXLanes>> chipInvoke;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const KeySoundFXInfo& fx);

	struct KeySoundLaserInfo
	{
		DefList<KeySoundDef> def;
		InvokeList<ByPulse<KeySoundParams>> slamEvent;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const KeySoundLaserInfo& laser);

	struct KeySoundInfo
	{
		KeySoundFXInfo fx;

		KeySoundLaserInfo laser;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const KeySoundInfo& keySound);
}
