#pragma once
#include "ksh/common/common.hpp"
#include "time_sig.hpp"

namespace ksh
{
	constexpr Pulse kDefaultResolution = 240;

	struct BeatMap
	{
		ByPulse<double> bpmChanges;

		ByMeasureIdx<TimeSig> timeSigChanges;

		Pulse resolution = kDefaultResolution;
	};

	void to_json(nlohmann::json& j, const BeatMap& beatMap);

	void from_json(const nlohmann::json& j, BeatMap& beatMap);
}
