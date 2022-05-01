#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct GaugeRoot
	{
		double total = 0.0;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const GaugeRoot& gauge);
}
