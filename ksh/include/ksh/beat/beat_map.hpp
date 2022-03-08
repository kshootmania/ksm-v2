#pragma once
#include "ksh/common/common.hpp"
#include "time_sig.hpp"

namespace ksh
{
	struct BeatMap
	{
		ByPulse<double> bpmChanges;

		ByMeasureIdx<TimeSig> timeSigChanges;

		Pulse resolution = 240;
	};
}
