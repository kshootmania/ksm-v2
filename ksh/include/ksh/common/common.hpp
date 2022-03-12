#pragma once
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <cstdint>

namespace ksh
{
	using String = std::u32string;
	using StringView = std::u32string_view;

	// Since the encoding of a KSH file can be either UTF-8 or Shift-JIS, char is used here instead of u32char.
	using StringC = std::string;
	using StringViewC = std::string_view;

	using Ms = double;
	using Pulse = std::int64_t;
	using RelPulse = std::int64_t;

	template <typename T>
	using ByPulse = std::map<Pulse, T>;

	template <typename T>
	using ByPulseMulti = std::multimap<Pulse, T>;

	template <typename T>
	using ByRelPulse = std::map<RelPulse, T>;

	template <typename T>
	using ByRelPulseMulti = std::multimap<RelPulse, T>;

	template <typename T>
	using ByMeasureIdx = std::map<std::int64_t, T>;

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

	struct RelGraphValue
	{
		double rv = 0.0;
		double rvf = 0.0;

		RelGraphValue(double rv)
			: rv(rv)
			, rvf(rv)
		{
		}

		RelGraphValue(double rv, double rvf)
			: rv(rv)
			, rvf(rvf)
		{
		}
	};

	using Graph = ByPulse<GraphValue>;

	using GraphSections = ByPulse<ByRelPulse<GraphValue>>;

	using RelGraph = ByPulse<RelGraphValue>;

	using RelGraphSections = ByRelPulse<RelGraphValue>;

	template <typename T>
	using Lane = ByPulse<T>; // TODO: For editor, ByPulseMulti would be used instead of ByPulse?

	template <typename T>
	using DefList = std::unordered_map<std::u8string, T>;

	template <typename T>
	using InvokeList = std::unordered_map<std::u8string, T>;

	constexpr std::size_t kNumBTLanes = 4;
	constexpr std::size_t kNumFXLanes = 2;
	constexpr std::size_t kNumLaserLanes = 2;
}
