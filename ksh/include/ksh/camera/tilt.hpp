#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct TiltRoot
	{
		GraphSections manualTilts;
		ByPulse<RelPulse> keepIntervals;

		bool empty() const
		{
			return manualTilts.empty() && keepIntervals.empty();
		}
	};

	inline void to_json(nlohmann::json& j, const TiltRoot& tilt)
	{
		if (!tilt.manualTilts.empty())
		{
			j["manual"] = tilt.manualTilts;
		}

		if (!tilt.keepIntervals.empty())
		{
			j["keep"] = tilt.keepIntervals;
		}
	}
}
