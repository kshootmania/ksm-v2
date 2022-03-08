#pragma once
#include <string>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <cstdint>

namespace ksh
{
	using String = std::u32string;

	using Ms = double;
	using Pulse = int64_t;
	using RelPulse = int64_t;

	template <typename T>
	using ByPulse = std::map<Pulse, T>;

	template <typename T>
	using ByPulseMulti = std::multimap<Pulse, T>;

	template <typename T>
	using ByRelPulse = std::map<RelPulse, T>;

	template <typename T>
	using ByRelPulseMulti = std::multimap<RelPulse, T>;

	template <typename T>
	using ByMeasureIdx = std::map<int64_t, T>;

	template <typename Params>
	struct ByBtnNote
	{
		std::size_t laneIdx = 0;
		std::size_t noteIdx = 0;
		Params params;
		//bool dom = true; // TODO: Only used in audio effects? Anyways, the value is always true in KSH.
	};

	template <typename Params>
	struct ByLaserNote
	{
		std::size_t laneIdx = 0;
		std::size_t sectionIdx = 0;
		std::size_t pointIdx = 0;
		Params params;
		//bool dom = true; // TODO: Only used in audio effects? Anyways, the value is always true in KSH.
	};

	template <typename Params>
	struct ByNotes
	{
		std::vector<ByBtnNote<Params>> bt;
		std::vector<ByBtnNote<Params>> fx;
		std::vector<ByLaserNote<Params>> laser;
	};

	struct Interval
	{
		RelPulse length = 0;
	};

	struct GraphValue
	{
		double value = 0.0;

		// More members will be added here in the kson format
	};

	struct RelGraphValue
	{
		double relativeValue = 0.0;

		// More members will be added here in the kson format
	};

	using Graph = ByPulse<GraphValue>;

	using GraphSections = ByPulse<ByRelPulse<GraphValue>>;

	using RelGraph = ByPulse<RelGraphValue>;

	using RelGraphSections = ByRelPulse<RelGraphValue>;

	template <typename T, std::size_t N>
	using Lane = std::array<ByPulse<T>, N>; // TODO: For editor, ByPulseMulti would be used instead of ByPulse?

	template <typename T>
	using DefList = std::unordered_map<String, T>;

	template <typename T>
	using InvokeList = std::unordered_map<String, T>;
}
