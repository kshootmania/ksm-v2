#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct KeySoundParams
	{
		double volume = 1.0;
	};

	struct KeySoundDef
	{
		KeySoundParams v;
	};

	struct KeySoundFXInfo
	{
		Dict<KeySoundDef> def;
		Dict<FXLane<KeySoundParams>> chipEvent;

		bool empty() const;
	};

	struct KeySoundLaserInfo
	{
		Dict<KeySoundDef> def;
		Dict<ByPulse<KeySoundParams>> slamEvent;

		bool empty() const;
	};

	struct KeySoundInfo
	{
		KeySoundFXInfo fx;
		KeySoundLaserInfo laser;

		bool empty() const;
	};
}
