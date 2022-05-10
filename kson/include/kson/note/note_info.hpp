#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct LaserSection
	{
		ByRelPulse<GraphValue> points;

		int8_t xScale = 1; // 1-2, sets whether the laser section is 2x-widen or not
	};

	void to_json(nlohmann::json& j, const Lane<LaserSection>& lane);

	struct NoteInfo
	{
		std::array<Lane<Interval>, kNumBTLanes> btLanes;
		std::array<Lane<Interval>, kNumFXLanes> fxLanes;
		std::array<Lane<LaserSection>, kNumLaserLanes> laserLanes;
	};

	void to_json(nlohmann::json& j, const NoteInfo& noteInfo);
}
