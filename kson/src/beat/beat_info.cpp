#include "kson/beat/beat_info.hpp"

void kson::to_json(nlohmann::json& j, const BeatInfo& beatInfo)
{
	nlohmann::json bpmChanges = nlohmann::json::array();
	for (const auto [y, bpm] : beatInfo.bpmChanges)
	{
		bpmChanges.push_back({
			{ "y", y },
			{ "v", bpm },
		});
	}

	j = {
		{ "bpm", bpmChanges },
		{ "time_sig", beatInfo.timeSigChanges },
	};

	if (beatInfo.resolution != kDefaultResolution)
	{
		j["resolution"] = beatInfo.resolution;
	}
}

void kson::from_json(const nlohmann::json& j, BeatInfo& beatInfo)
{
	j.at("bpm").get_to(beatInfo.bpmChanges);
	j.at("time_sig").get_to(beatInfo.timeSigChanges);
	if (j.contains("resolution"))
	{
		j.at("resolution").get_to(beatInfo.resolution);
	}
	else
	{
		beatInfo.resolution = kDefaultResolution;
	}
}
