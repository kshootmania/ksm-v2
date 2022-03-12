#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct LaserSection
	{
		ByRelPulse<GraphValue> points;

		int8_t xScale = 1; // 1-2, sets whether the laser section is 2x-widen or not
	};

	struct NoteRoot
	{
		std::array<Lane<RelPulse>, 4> btLanes;
		std::array<Lane<RelPulse>, 2> fxLanes;
		std::array<Lane<LaserSection>, 2> laserLanes;
	};
}
