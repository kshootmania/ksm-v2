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
		Dict<KeySoundDef> def;
		Dict<std::array<ByPulse<KeySoundParams>, kNumFXLanes>> chipEvent;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const KeySoundFXInfo& fx);

	struct KeySoundLaserInfo
	{
		Dict<KeySoundDef> def;
		Dict<ByPulse<KeySoundParams>> slamEvent;

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
