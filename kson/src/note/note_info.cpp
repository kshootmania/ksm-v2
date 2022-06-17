#include "kson/note/note_info.hpp"

void kson::to_json(nlohmann::json& j, const ByPulse<LaserSection>& lane)
{
	j = nlohmann::json::array();

	for (const auto& [y, section] : lane)
	{
		nlohmann::json sectionJSON = {
			{ "y", y },
			{ "v", ByRelPulseToJSON(section.points) },
		};

		if (section.w != 1)
		{
			sectionJSON["w"] = section.w;
		}

		j.push_back(std::move(sectionJSON));
	}
}

void kson::to_json(nlohmann::json& j, const NoteInfo& note)
{
	nlohmann::json bt = nlohmann::json::array();
	for (const auto& lane : note.bt)
	{
		bt.push_back(lane);
	}

	nlohmann::json fx = nlohmann::json::array();
	for (const auto& lane : note.fx)
	{
		fx.push_back(lane);
	}

	j = {
		{ "bt", bt },
		{ "fx", fx },
		{ "laser", note.laser },
	};
}
