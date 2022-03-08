#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct TiltRoot
	{
		GraphSections manualTilts;
		ByPulse<Interval> keepIntervals;
	};
}
