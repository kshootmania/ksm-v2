#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct LaserSection
	{
		ByRelPulse<GraphValue> points;

		int8_t xScale = 1; // 1-2, sets whether the laser section is 2x-widen or not
	};

	inline void to_json(nlohmann::json& j, const Lane<LaserSection>& lane)
	{
		j = nlohmann::json::array();

		for (const auto& [y, section] : lane)
		{
			nlohmann::json sectionJSON = {
				{ "y", y },
				{ "v", section.points },
			};

			if (section.xScale != 1)
			{
				sectionJSON["w"] = section.xScale;
			}

			j.push_back(std::move(sectionJSON));
		}
	}

	struct NoteRoot
	{
		std::array<Lane<Interval>, 4> btLanes;
		std::array<Lane<Interval>, 2> fxLanes;
		std::array<Lane<LaserSection>, 2> laserLanes;
	};

	inline void to_json(nlohmann::json& j, const NoteRoot& noteRoot)
	{
		nlohmann::json bt = nlohmann::json::array();
		for (const auto& lane : noteRoot.btLanes)
		{
			bt.push_back(lane);
		}

		nlohmann::json fx = nlohmann::json::array();
		for (const auto& lane : noteRoot.fxLanes)
		{
			fx.push_back(lane);
		}

		j = {
			{ "bt", bt },
			{ "fx", fx },
			{ "laser", noteRoot.laserLanes },
		};
	}
}
