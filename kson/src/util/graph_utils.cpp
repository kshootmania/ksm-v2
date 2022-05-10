#include "kson/util/graph_utils.hpp"

double kson::GraphValueAt(const Graph& graph, Pulse pulse)
{
	if (graph.empty())
	{
		return 0.0;
	}

	auto itr = graph.upper_bound(pulse);
	if (itr != graph.begin())
	{
		--itr;
	}

	const auto& [pulse1, value1] = *itr;
	if (pulse < pulse1)
	{
		return value1.v;
	}
	if (itr == graph.end())
	{
		return value1.vf;
	}

	const auto& [pulse2, value2] = *std::next(itr);
	assert(pulse1 <= pulse && pulse < pulse2);

	const double lerpRate = static_cast<double>(pulse - pulse1) / static_cast<double>(pulse2 - pulse1);
	return Lerp(value1.vf, value2.v, lerpRate);
}

kson::GraphSections::const_iterator kson::GraphSectionAt(const GraphSections& graphSections, Pulse pulse)
{
	assert(!graphSections.empty());

	auto itr = graphSections.upper_bound(pulse);
	if (itr != graphSections.begin())
	{
		--itr;
	}

	return itr;
}

kson::Lane<kson::LaserSection>::const_iterator kson::LaserSectionAt(const Lane<LaserSection>& laserSections, Pulse pulse)
{
	assert(!laserSections.empty());

	auto itr = laserSections.upper_bound(pulse);
	if (itr != laserSections.begin())
	{
		--itr;
	}

	return itr;
}
