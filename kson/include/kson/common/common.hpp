#pragma once
#pragma warning(disable:26819) // Ignore warnings in nlohmann/json
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <cassert>
#include <cstdint>
#include "kson/encoding/encoding.hpp"
#include "kson/third_party/nlohmann/json.hpp"

namespace kson
{
	constexpr std::size_t kNumBTLanes = 4;
	constexpr std::size_t kNumFXLanes = 2;
	constexpr std::size_t kNumLaserLanes = 2;

	using Ms = double;
	using Pulse = std::int64_t;
	using RelPulse = std::int64_t;

	// The difference between Pulse and RelPulse is only for annotation
	static_assert(std::is_same_v<Pulse, RelPulse>);

	template <typename T>
	using ByPulse = std::map<Pulse, T>;

	template <typename T>
	void to_json(nlohmann::json& j, const ByPulse<T>& events)
	{
		j = nlohmann::json::array();

		for (const auto& [y, v] : events)
		{
			if (nlohmann::json(v).empty())
			{
				j.push_back({
					{ "y", y },
				});
			}
			else
			{
				j.push_back({
					{ "y", y },
					{ "v", v },
				});
			}
		}
	}

	template <typename T>
	using ByPulseMulti = std::multimap<Pulse, T>;

	template <typename T>
	void to_json(nlohmann::json& j, const ByPulseMulti<T>& events)
	{
		j = nlohmann::json::array();

		for (const auto& [y, v] : events)
		{
			if (nlohmann::json(v).empty())
			{
				j.push_back({
					{ "y", y },
				});
			}
			else
			{
				j.push_back({
					{ "y", y },
					{ "v", v },
				});
			}
		}
	}

	template <typename T>
	using ByRelPulse = std::map<RelPulse, T>;

	// Note: This function cannot overload to_json() because ByRelPulse<T> is the same type as ByPulse<T>
	template <typename T>
	nlohmann::json ByRelPulseToJSON(const ByRelPulse<T>& events)
	{
		nlohmann::json j = nlohmann::json::array();

		for (const auto& [ry, v] : events)
		{
			if (nlohmann::json(v).empty())
			{
				j.push_back({
					{ "ry", ry },
				});
			}
			else
			{
				j.push_back({
					{ "ry", ry },
					{ "v", v },
				});
			}
		}

		return j;
	}

	template <typename T>
	using ByRelPulseMulti = std::multimap<RelPulse, T>;

	// Note: This function cannot overload to_json() because ByRelPulseMulti<T> is the same type as ByPulseMulti<T>
	template <typename T>
	nlohmann::json ByRelPulseMultiToJSON(const ByRelPulseMulti<T>& events)
	{
		nlohmann::json j = nlohmann::json::array();

		for (const auto& [ry, v] : events)
		{
			if (nlohmann::json(v).empty())
			{
				j.push_back({
					{ "ry", ry },
				});
			}
			else
			{
				j.push_back({
					{ "ry", ry },
					{ "v", v },
				});
			}
		}

		return j;
	}

	template <typename T>
	using ByMeasureIdx = std::map<std::int64_t, T>;

	template <typename T>
	void ByMeasureIdxToJSON(nlohmann::json& j, const ByMeasureIdx<T>& events)
	{
		j = nlohmann::json::array();

		for (const auto& [idx, v] : events)
		{
			if (nlohmann::json(v).empty())
			{
				j.push_back({
					{ "idx", idx },
				});
			}
			else
			{
				j.push_back({
					{ "idx", idx },
					{ "v", v },
				});
			}
		}
	}

	template <typename Params>
	struct ByNotes
	{
		std::array<ByPulse<Params>, kNumBTLanes> bt;
		std::array<ByPulse<Params>, kNumFXLanes> fx;
		std::array<ByPulse<Params>, kNumLaserLanes> laser;
	};

	template <typename T>
	void to_json(nlohmann::json& j, const ByNotes<T>& byNotes)
	{
		j = nlohmann::json::object();

		if (nlohmann::json bt = byNotes.bt; !bt[0].empty() || !bt[1].empty() || !bt[2].empty() || !bt[3].empty())
		{
			j["bt"] = byNotes.bt;
		}

		if (nlohmann::json fx = byNotes.fx; !fx[0].empty() || !fx[1].empty())
		{
			j["fx"] = byNotes.fx;
		}

		if (nlohmann::json laser = byNotes.laser; !laser[0].empty() || !laser[1].empty())
		{
			j["laser"] = byNotes.laser;
		}
	}

	struct GraphValue
	{
		double v = 0.0;
		double vf = 0.0;

		GraphValue(double v)
			: v(v)
			, vf(v)
		{
		}

		GraphValue(double v, double vf)
			: v(v)
			, vf(vf)
		{
		}
	};

	void to_json(nlohmann::json& j, const GraphValue& graphValue);

	struct Interval
	{
		RelPulse length = 0;
	};

	void to_json(nlohmann::json& j, const ByPulse<Interval>& intervals);

	// Note: This function cannot overload to_json() because ByRelPulse<T> is the same type as ByPulse<T>
	void IntervalByRelPulseToJSON(nlohmann::json& j, const ByRelPulse<Interval>& intervals);

	void to_json(nlohmann::json& j, const ByPulseMulti<Interval>& intervals);

	// Note: This function cannot overload to_json() because ByRelPulseMulti<T> is the same type as ByPulseMulti<T>
	void IntervalByRelPulseMultiToJSON(nlohmann::json& j, const ByRelPulseMulti<Interval>& intervals);

	using Graph = ByPulse<GraphValue>;

	using GraphSections = ByPulse<ByRelPulse<GraphValue>>;

	template <typename T>
	using Lane = ByPulse<T>; // TODO: For editor, ByPulseMulti would be used instead of ByPulse?

	template <typename T>
	using DefList = std::unordered_map<std::string, T>;

	template <typename T>
	using InvokeList = std::unordered_map<std::string, T>;

	template <typename T>
	T Lerp(T value1, T value2, double rate)
	{
		return value1 + static_cast<T>((value2 - value1) * rate);
	}

	template <typename T, typename U>
	auto CurrentAt(const std::map<T, U>& map, T key)
	{
		auto itr = map.upper_bound(key);
		if (itr != map.begin())
		{
			--itr;
		}
		return itr;
	}
}
