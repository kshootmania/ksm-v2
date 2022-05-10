#include "kson/note/note.hpp"

void kson::to_json(nlohmann::json& j, const Lane<LaserSection>& lane)
{
	j = nlohmann::json::array();

	for (const auto& [y, section] : lane)
	{
		nlohmann::json sectionJSON = {
			{ "y", y },
			{ "v", ByRelPulseToJSON(section.points) },
		};

		if (section.xScale != 1)
		{
			sectionJSON["w"] = section.xScale;
		}

		j.push_back(std::move(sectionJSON));
	}
}

void kson::to_json(nlohmann::json& j, const NoteRoot& noteRoot)
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
