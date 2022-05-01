#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct TiltRoot
	{
		GraphSections manualTilts;
		ByPulse<bool> keep;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const TiltRoot& tilt);
}
