#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct GaugeInfo
	{
		double total = 0.0;

		bool empty() const;
	};
}
