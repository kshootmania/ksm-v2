#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct GaugeRoot
	{
		double total = 0.0;

		bool empty() const
		{
			return total == 0.0;
		}
	};

	inline void to_json(nlohmann::json& j, const GaugeRoot& gauge)
	{
		j = nlohmann::json::object();

		if (gauge.total != 0.0)
		{
			j["total"] = gauge.total;
		}
	}
}
