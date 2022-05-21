#include "kson/beat/beat_info.hpp"

void kson::to_json(nlohmann::json& j, const BeatInfo& beatInfo)
{
	nlohmann::json bpm = nlohmann::json::array();
	for (const auto [y, v] : beatInfo.bpm)
	{
		bpm.push_back({
			{ "y", y },
			{ "v", v },
		});
	}

	j = {
		{ "bpm", bpm },
	};

	const auto itr = beatInfo.timeSig.begin();
	if (beatInfo.timeSig.size() != 1U || itr->first != 0 || itr->second.numerator != 4 || itr->second.denominator != 4)
	{
		j["time_sig"] = beatInfo.timeSig;
	}
}

void kson::from_json(const nlohmann::json& j, BeatInfo& beatInfo)
{
	j.at("bpm").get_to(beatInfo.bpm);
	j.at("time_sig").get_to(beatInfo.timeSig);
}
