#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct CamGraphs
	{
		Graph zoom; // "zoom_bottom"
		Graph shiftX; // "zoom_side"
		Graph rotationX; // "zoom_top"
		Graph rotationZ; // Rotation degree
		Graph rotationZHighway; // Rotation degree (highway only)
		Graph rotationZJdgline; // Rotation degree (judgment line only)
		Graph centerSplit; // "center_split"
	};

	struct CamPatternParams
	{
		RelPulse length = 0;
		double scale = 1.0;  // Swing only
		std::int32_t repeat = 1;  // Swing only
		std::int32_t decayOrder = 0;  // Swing only
	};

	namespace CamPatternKey
	{
		constexpr std::string_view kSpin = "spin";
		constexpr std::string_view kHalfSpin = "half_spin";
		constexpr std::string_view kSwing = "swing";
	}

	template <typename T>
	struct WithDirection
	{
		T v;
		std::int32_t d = 0; // laser slam direction, -1 (left) or 1 (right)
	};

	struct CamPatternLaserInfo
	{
		Dict<ByPulse<WithDirection<CamPatternParams>>> slamEvent;
	};

	struct CamPatternInfo
	{
		CamPatternLaserInfo laser;
	};

	struct CamInfo
	{
		CamGraphs body;
		CamPatternInfo pattern;
	};
}
