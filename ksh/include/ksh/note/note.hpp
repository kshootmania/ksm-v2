#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct LaserSection
	{
		GraphSections points;

		int8_t xScale = 1; // 1-2, sets whether the laser section is 2x-widen or not
	};

	struct NoteRoot
	{
		Lane<Interval, 4> btLanes;
		Lane<Interval, 2> fxLanes;
		Lane<LaserSection, 2> laserLanes;
	};
}
