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
		std::array<ByPulse<Interval>, kNumBTLanes> btLanes;
		std::array<ByPulse<Interval>, kNumFXLanes> fxLanes;
		std::array<ByPulse<LaserSection>, kNumLaserLanes> laserLanes;
	};

	void to_json(nlohmann::json& j, const NoteInfo& noteInfo);
}
