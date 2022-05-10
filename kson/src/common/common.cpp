#include "kson/common/common.hpp"

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

void kson::to_json(nlohmann::json& j, const RelGraphValue& graphValue)
{
	if (graphValue.rv == graphValue.rvf)
	{
		j = {
			{ "rv", graphValue.rv },
		};
	}
	else
	{
		j = {
			{ "rv", graphValue.rv },
			{ "rvf", graphValue.rvf },
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
