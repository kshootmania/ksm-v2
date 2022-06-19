#pragma once
#include "kson/common/common.hpp"
#include "time_sig.hpp"

namespace kson
{
	struct BeatInfo
	{
		ByPulse<double> bpm;

		ByMeasureIdx<TimeSig> timeSig;
	};
}
