#include "kson/common/common.hpp"
#include <optional>

void kson::to_json(nlohmann::json& j, const GraphValue& graphValue)
{
	if (graphValue.v == graphValue.vf)
	{
		j = {
			{ "v", graphValue.v },
		};
	}
	else
	{
		j = {
			{ "v", graphValue.v },
			{ "vf", graphValue.vf },
		};
	}
}

void kson::to_json(nlohmann::json& j, const ByPulse<Interval>& intervals)
{
	j = nlohmann::json::array();

	for (const auto& [y, interval] : intervals)
	{
		if (interval.length == 0)
		{
			j.push_back({
				{ "y", y },
			});
		}
		else
		{
			j.push_back({
				{ "y", y },
				{ "l", interval.length },
			});
		}
	}
}

void kson::IntervalByRelPulseToJSON(nlohmann::json& j, const ByRelPulse<Interval>& intervals)
{
	j = nlohmann::json::array();

	for (const auto& [y, interval] : intervals)
	{
		if (interval.length == 0)
		{
			j.push_back({
				{ "ry", y },
			});
		}
		else
		{
			j.push_back({
				{ "ry", y },
				{ "l", interval.length },
			});
		}
	}
}

void kson::to_json(nlohmann::json& j, const ByPulseMulti<Interval>& intervals)
{
	j = nlohmann::json::array();

	for (const auto& [y, interval] : intervals)
	{
		if (interval.length == 0)
		{
			j.push_back({
				{ "y", y },
			});
		}
		else
		{
			j.push_back({
				{ "y", y },
				{ "l", interval.length },
			});
		}
	}
}

void kson::GraphSectionsToJSON(nlohmann::json& j, const GraphSections& graphSections)
{
	j = nlohmann::json::array();
	for (const auto& [y, graphSection] : graphSections)
	{
		auto graphSectionJSON = nlohmann::json::array();
		std::optional<double> prevValue = std::nullopt;
		for (const auto& [ry, v] : graphSection)
		{
			nlohmann::json graphPointJSON = {
				{ "ry", ry },
			};
			if (v.v != prevValue)
			{
				graphPointJSON["v"] = v.v;
			}
			if (v.vf != v.v)
			{
				graphPointJSON["vf"] = v.vf;
			}
			graphSectionJSON.push_back(graphPointJSON);
			prevValue = v.v;
		}
		j.push_back({
			{ "y", y },
			{ "v", graphSectionJSON },
		});
	}
}

// Note: This function cannot overload to_json() because ByRelPulseMulti<T> is the same type as ByPulseMulti<T>
void kson::IntervalByRelPulseMultiToJSON(nlohmann::json& j, const ByRelPulseMulti<Interval>& intervals)
{
	j = nlohmann::json::array();

	for (const auto& [y, interval] : intervals)
	{
		if (interval.length == 0)
		{
			j.push_back({
				{ "ry", y },
			});
		}
		else
		{
			j.push_back({
				{ "ry", y },
				{ "l", interval.length },
			});
		}
	}
}
