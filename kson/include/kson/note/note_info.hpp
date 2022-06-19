#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct LaserSection
	{
		ByRelPulse<GraphValue> points;

		std::int32_t w = 1; // 1-2, sets whether the laser section is 2x-widen or not
	};

	struct NoteInfo
	{
		BTLane<Interval> bt;
		FXLane<Interval> fx;
		LaserLane<LaserSection> laser;
	};
}
