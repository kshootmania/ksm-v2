#pragma once
#include <optional>
#include "kson/common/common.hpp"
#include "kson/note/note_info.hpp"

namespace kson
{
	double GraphValueAt(const Graph& graph, Pulse pulse);

	GraphSections::const_iterator GraphSectionAt(const GraphSections& graphSections, Pulse pulse);

	ByPulse<LaserSection>::const_iterator LaserSectionAt(const ByPulse<LaserSection>& laserSections, Pulse pulse);

	template <class GS>
	std::optional<double> GraphSectionValueAt(const GS& graphSections, Pulse pulse) requires std::is_same_v<GS, GraphSections> || std::is_same_v<GS, ByPulse<LaserSection>>
	{
		if (graphSections.empty())
		{
			return std::nullopt;
		}

		const Graph* pGraphSection;
		RelPulse relPulse;
		if constexpr (std::is_same_v<GS, ByPulse<LaserSection>>)
		{
			const auto& [laserSectionPulse, laserSection] = *LaserSectionAt(graphSections, pulse);
			relPulse = pulse - laserSectionPulse;
			pGraphSection = &laserSection.points;
		}
		else
		{
			const auto& [graphSectionPulse, graphSection] = *GraphSectionAt(graphSections, pulse);
			relPulse = pulse - graphSectionPulse;
			pGraphSection = &graphSection;
		}

		if (pGraphSection->size() <= 1)
		{
			return std::nullopt;
		}

		{
			const auto& [firstRelPulse, _] = *pGraphSection->begin();
			if (relPulse < firstRelPulse)
			{
				return std::nullopt;
			}
		}

		{
			const auto& [lastRelPulse, _] = *pGraphSection->rbegin();
			if (relPulse >= lastRelPulse)
			{
				return std::nullopt;
			}
		}

		return std::make_optional(GraphValueAt(*pGraphSection, relPulse));
	}

	template <class GS>
	double GraphSectionValueAtWithDefault(const GS& graphSections, Pulse pulse, double defaultValue) requires std::is_same_v<GS, GraphSections> || std::is_same_v<GS, ByPulse<LaserSection>>
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
