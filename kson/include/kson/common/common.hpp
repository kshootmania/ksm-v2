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

	constexpr Pulse kResolution = 240;
	constexpr Pulse kResolution4 = kResolution * 4;

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
	using Dict = std::unordered_map<std::string, T>;

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

	template <typename T>
	std::size_t CountInRange(const ByPulse<T>& map, Pulse start, Pulse end)
	{
		static_assert(std::is_signed_v<Pulse>);
		assert(start <= end);

		const auto itr1 = map.upper_bound(start - Pulse{ 1 });
		if (itr1 == map.end() || itr1->first >= end)
		{
			return std::size_t{ 0 };
		}
		const auto itr2 = map.upper_bound(end - Pulse{ 1 });
		return static_cast<std::size_t>(std::distance(itr1, itr2));
	}

	template <typename T>
	auto FirstInRange(const ByPulse<T>& map, Pulse start, Pulse end)
	{
		static_assert(std::is_signed_v<Pulse>);
		assert(start <= end);

		const auto itr = map.upper_bound(start - Pulse{ 1 });
		if (itr == map.end() || itr->first >= end)
		{
			return map.end();
		}
		return itr;
	}
}
