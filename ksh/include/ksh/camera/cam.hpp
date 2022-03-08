#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	template <typename G> // G: Graph or RelGraph
	struct CamGraphs
	{
		G zoom; // "zoom_bottom"
		G shiftX; // "zoom_side"
		G rotationX; // "zoom_top"
		G rotationZ; // Rotation degree
		G rotationZLane; // Rotation degree (lane only)
		G rotationZJdgLine; // Rotation degree (judgment line only)
		G centerSplit; // "center_split"
	};

	struct CamPatternParams
	{
		RelPulse length;
		double scale = 1.0;
		int64_t repeat = 1;
		double repeatScale = 1.0;
		double decayOrder = 0.0;
	};

	struct CamPatternDef
	{
		CamGraphs<RelGraph> body;
		CamPatternParams params;
	};

	struct CamPatternInfo
	{
		DefList<CamPatternDef> defList;
		InvokeList<ByPulse<CamPatternParams>> pulseEventList;
		InvokeList<ByNotes<CamPatternParams>> noteEventList;
	};

	struct CamRoot
	{
		CamGraphs<Graph> body;
		CamGraphs<Graph> tiltAssignScale;
		CamPatternInfo patternInfo;
	};
}
