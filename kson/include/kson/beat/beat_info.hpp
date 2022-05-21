#pragma once
#include "kson/common/common.hpp"
#include "time_sig.hpp"

namespace kson
{
	struct BeatInfo
	{
		ByPulse<double> bpmChanges;

		ByMeasureIdx<TimeSig> timeSigChanges;

		Pulse resolution = kResolution; // TODO: delete this
	};

	void to_json(nlohmann::json& j, const BeatInfo& beatInfo);

	void from_json(const nlohmann::json& j, BeatInfo& beatInfo);
}
