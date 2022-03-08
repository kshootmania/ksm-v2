#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct KSHRotationFlags
	{
		bool tiltAffected = false;
		bool spinAffected = false;
	};

	struct KSHBGInfo
	{
		String filename;
		KSHRotationFlags rotationFlags = { true, false };
	};

	struct KSHLayerInfo
	{
		String filename;
		int64_t durationMs = 0;
		KSHRotationFlags rotationFlags = { true, true };
	};

	struct KSHMovieInfo
	{
		String filename;
		int64_t offsetMs = 0;
	};

	struct LegacyBGRoot
	{
		// first index: when gauge < 70%, second index: when gauge >= 70%
		std::array<KSHBGInfo, 2> bgInfos;
		std::array<KSHLayerInfo, 2> layerInfos;

		KSHMovieInfo movieInfos;
	};

	struct BGRoot
	{
		// KSH format will be legacy
		LegacyBGRoot legacy;
	};
}
