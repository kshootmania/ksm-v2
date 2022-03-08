#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct LegacyAudioBGMInfo
	{
		// Filenames of pre-rendered BGM with audio effects from legacy KSH charts
		String filenameF; // "xxx_f.ogg"
		String filenameP; // "xxx_p.ogg"
		String filenameFP; // "xxx_fp.ogg"
	};

	struct LegacyAudioInfo
	{
		bool laserSlamAutoVolume = false; // "chokkakuautovol" in KSH

		LegacyAudioBGMInfo legacyBGMInfo;
	};
}
