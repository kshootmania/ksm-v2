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

		bool operator==(const KSHBGInfo& rhs) const;
	};

	struct KSHLayerInfo
	{
		std::string filename; // UTF-8 guaranteed
		std::int32_t duration = 0;
		KSHLayerRotationInfo rotation = { true, true };

		bool empty() const;
	};

	struct KSHMovieInfo
	{
		std::string filename; // UTF-8 guaranteed
		std::int32_t offset = 0;

		bool empty() const;
	};

	struct LegacyBGInfo
	{
		// first index: when gauge < 70%, second index: when gauge >= 70%
		std::array<KSHBGInfo, 2> bg;

		KSHLayerInfo layer;

		KSHMovieInfo movie;

		bool empty() const;
	};

	struct BGInfo
	{
		// KSH format will be legacy
		LegacyBGInfo legacy;
	};
}
