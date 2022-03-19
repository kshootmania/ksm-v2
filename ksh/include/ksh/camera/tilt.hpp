#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct TiltRoot
	{
		GraphSections manualTilts;
		ByPulse<bool> keep;

		bool empty() const
		{
			return manualTilts.empty() && keep.empty();
		}
	};

	inline void to_json(nlohmann::json& j, const TiltRoot& tilt)
	{
		j = nlohmann::json::object();

		if (!tilt.manualTilts.empty())
		{
			j["manual"] = tilt.manualTilts;
		}

		if (!tilt.keep.empty())
		{
			auto keep = nlohmann::json::array();

			// Skip the same value
			// (The first "false" is also skipped)
			bool prevValue = false;
			for (const auto& [y, v] : tilt.keep)
			{
				if (v == prevValue)
				{
					continue;
				}

				keep.push_back({
					{ "y", y },
					{ "v", v },
				});

				prevValue = v;
			}

			j["keep"] = keep;
		}
	}
}
