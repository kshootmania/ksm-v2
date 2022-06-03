#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct LaserSection
	{
		ByRelPulse<GraphValue> points;

		std::int32_t xScale = 1; // 1-2, sets whether the laser section is 2x-widen or not
	};

	void to_json(nlohmann::json& j, const ByPulse<LaserSection>& lane);

	struct NoteInfo
	{
		BTLane<Interval> btLanes;
		FXLane<Interval> fxLanes;
		LaserLane<LaserSection> laserLanes;
	};

	void to_json(nlohmann::json& j, const NoteInfo& noteInfo);
}
