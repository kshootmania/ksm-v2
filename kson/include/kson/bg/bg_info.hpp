#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct KSHLayerRotationInfo
	{
		bool tilt = false;
		bool spin = false;
	};

	struct KSHBGInfo
	{
		std::string filename; // UTF-8 guaranteed
	};

	struct KSHLayerInfo
	{
		std::string filename; // UTF-8 guaranteed
		std::int32_t duration = 0;
		KSHLayerRotationInfo rotation = { true, true };
	};

	struct KSHMovieInfo
	{
		std::string filename; // UTF-8 guaranteed
		std::int32_t offset = 0;
	};

	struct LegacyBGInfo
	{
		std::array<KSHBGInfo, 2> bg; // first index: when gauge < 70%, second index: when gauge >= 70%
		KSHLayerInfo layer;
		KSHMovieInfo movie;
	};

	struct BGInfo
	{
		// KSMv1 BG format will be legacy!
		LegacyBGInfo legacy;
	};
}
