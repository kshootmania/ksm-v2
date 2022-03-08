#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct KeySoundParams
	{
		double volume = 1.0;
	};

	struct KeySoundDef
	{
		String filename;

		KeySoundParams defaultParams;
	};

	struct KeySoundRoot
	{
		DefList<KeySoundDef> defList;
		InvokeList<ByPulse<KeySoundParams>> pulseEventList;
		InvokeList<ByNotes<KeySoundParams>> noteEventList;
	};
}
