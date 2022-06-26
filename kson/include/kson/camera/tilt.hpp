#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct TiltInfo
	{
		ByPulse<double> scale;
		ByPulse<GraphSection> manual;
		ByPulse<bool> keep;
	};
}
