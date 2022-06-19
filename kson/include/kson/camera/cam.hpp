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
		Graph rotationZLane; // Rotation degree (lane only)
		Graph rotationZJdgLine; // Rotation degree (judgment line only)
		Graph centerSplit; // "center_split"

		bool empty() const;
	};

	struct CamPatternParams
	{
		RelPulse length = 0;
		double scale = 1.0;  // Swing only
		std::int32_t repeat = 1;  // Swing only
		double decayOrder = 0.0;  // Swing only
	};

	struct CamPatternLaserInfo
	{
		Dict<ByPulse<CamPatternParams>> slamEvent;

		bool empty() const;
	};

	struct CamPatternInfo
	{
		CamPatternLaserInfo laser;

		bool empty() const;
	};

	struct CamInfo
	{
		CamGraphs body;
		CamPatternInfo pattern;

		bool empty() const;
	};
}
