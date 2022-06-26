#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	static constexpr std::int32_t kLaserXScale1x = 1;
	static constexpr std::int32_t kLaserXScale2x = 2;

	struct LaserSection
	{
		ByRelPulse<GraphValue> v; // Laser points

		std::int32_t w = kLaserXScale1x; // 1-2, sets whether the laser section is 2x-widen or not
	};

	struct NoteInfo
	{
		BTLane<Interval> bt;
		FXLane<Interval> fx;
		LaserLane<LaserSection> laser;
	};
}
