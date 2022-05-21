#pragma once
#include "kson/common/common.hpp"
#include "time_sig.hpp"

namespace kson
{
	struct BeatInfo
	{
		ByPulse<double> bpm;

		ByMeasureIdx<TimeSig> timeSig;
	};

	void to_json(nlohmann::json& j, const BeatInfo& beatInfo);

	void from_json(const nlohmann::json& j, BeatInfo& beatInfo);
}
