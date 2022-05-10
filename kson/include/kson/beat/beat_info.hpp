#pragma once
#include "kson/common/common.hpp"
#include "time_sig.hpp"

namespace kson
{
	constexpr Pulse kDefaultResolution = 240;

	struct BeatInfo
	{
		ByPulse<double> bpmChanges;

		ByMeasureIdx<TimeSig> timeSigChanges;

		Pulse resolution = kDefaultResolution;
	};

	void to_json(nlohmann::json& j, const BeatInfo& beatInfo);

	void from_json(const nlohmann::json& j, BeatInfo& beatInfo);
}
