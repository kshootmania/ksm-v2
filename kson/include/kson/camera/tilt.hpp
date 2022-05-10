#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct TiltInfo
	{
		ByPulse<double> scale;
		GraphSections manual;
		ByPulse<bool> keep;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const TiltInfo& tilt);
}
