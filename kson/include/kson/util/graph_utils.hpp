#pragma once
#include <optional>
#include "kson/common/common.hpp"
#include "kson/note/note_info.hpp"

namespace kson
{
	double GraphValueAt(const Graph& graph, Pulse pulse);

	template <class GS>
	ByPulse<GS>::const_iterator GraphSectionAt(const ByPulse<GS>& graphSections, Pulse pulse) requires std::is_same_v<GS, GraphSection> || std::is_same_v<GS, LaserSection>
	{
		assert(!graphSections.empty());

		auto itr = graphSections.upper_bound(pulse);
		if (itr != graphSections.begin())
		{
			--itr;
		}

		return itr;
	}

	template <class GS>
	std::optional<double> GraphSectionValueAt(const ByPulse<GS>& graphSections, Pulse pulse) requires std::is_same_v<GS, GraphSection> || std::is_same_v<GS, LaserSection>
	{
		if (graphSections.empty())
		{
			return std::nullopt;
		}

		const auto& [y, graphSection] = *GraphSectionAt(graphSections, pulse);
		const RelPulse ry = pulse - y;

		if (graphSection.v.size() <= 1)
		{
			return std::nullopt;
		}

		{
			const auto& [firstRy, _] = *graphSection.v.begin();
			if (ry < firstRy)
			{
				return std::nullopt;
			}
		}

		{
			const auto& [lastRy, _] = *graphSection.v.rbegin();
			if (ry >= lastRy)
			{
				return std::nullopt;
			}
		}

		return std::make_optional(GraphValueAt(graphSection.v, ry));
	}

	template <class GS>
	double GraphSectionValueAtWithDefault(const ByPulse<GS>& graphSections, Pulse pulse, double defaultValue) requires std::is_same_v<GS, GraphSection> || std::is_same_v<GS, LaserSection>
	{
		const std::optional<double> sectionValue = GraphSectionValueAt(graphSections, pulse);
		if (sectionValue.has_value())
		{
			return *sectionValue;
		}
		else
		{
			return defaultValue;
		}
	}
}
