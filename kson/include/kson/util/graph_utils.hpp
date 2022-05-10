#pragma once
#include <optional>
#include "kson/common/common.hpp"
#include "kson/note/note_info.hpp"

namespace kson
{
	namespace detail
	{
		inline double GenericGraphValueV(const GraphValue& graphValue)
		{
			return graphValue.v;
		}

		inline double GenericGraphValueV(const RelGraphValue& relGraphValue)
		{
			return relGraphValue.rv;
		}

		inline double GenericGraphValueVf(const GraphValue& graphValue)
		{
			return graphValue.vf;
		}

		inline double GenericGraphValueVf(const RelGraphValue& relGraphValue)
		{
			return relGraphValue.rvf;
		}
	}

	template <class G>
	double GraphValueAt(const G& graph, Pulse pulse) requires std::is_same_v<G, Graph> || std::is_same_v<G, RelGraph>
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
			return detail::GenericGraphValueV(value1);
		}
		if (itr == graph.end())
		{
			return detail::GenericGraphValueVf(value1);
		}

		const auto& [pulse2, value2] = *std::next(itr);
		assert(pulse1 <= pulse && pulse < pulse2);

		const double lerpRate = static_cast<double>(pulse - pulse1) / static_cast<double>(pulse2 - pulse1);
		return Lerp(detail::GenericGraphValueVf(value1), detail::GenericGraphValueV(value2), lerpRate);
	}

	template <class GS>
	const auto& GraphSectionAt(const GS& graphSections, Pulse pulse) requires std::is_same_v<GS, GraphSections> || std::is_same_v<GS, RelGraphSections>
	{
		assert(!graphSections.empty());

		auto itr = graphSections.upper_bound(pulse);
		if (itr != graphSections.begin())
		{
			--itr;
		}

		return *itr;
	}

	inline const auto& LaserSectionAt(const Lane<LaserSection>& laserSections, Pulse pulse)
	{
		assert(!laserSections.empty());

		auto itr = laserSections.upper_bound(pulse);
		if (itr != laserSections.begin())
		{
			--itr;
		}

		return *itr;
	}

	template <class GS>
	std::optional<double> GraphSectionValueAt(const GS& graphSections, Pulse pulse) requires std::is_same_v<GS, GraphSections> || std::is_same_v<GS, RelGraphSections> || std::is_same_v<GS, Lane<LaserSection>>
	{
		if (graphSections.empty())
		{
			return std::nullopt;
		}

		const Graph* pGraphSection;
		RelPulse relPulse;
		if constexpr (std::is_same_v<GS, Lane<LaserSection>>)
		{
			const auto& [laserSectionPulse, laserSection] = LaserSectionAt(graphSections, pulse);
			relPulse = pulse - laserSectionPulse;
			pGraphSection = &laserSection.points;
		}
		else
		{
			const auto& [graphSectionPulse, graphSection] = GraphSectionAt(graphSections, pulse);
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
	double GraphSectionValueAtWithDefault(const GS& graphSections, Pulse pulse, double defaultValue) requires std::is_same_v<GS, GraphSections> || std::is_same_v<GS, RelGraphSections> || std::is_same_v<GS, RelGraphSections> || std::is_same_v<GS, Lane<LaserSection>>
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
