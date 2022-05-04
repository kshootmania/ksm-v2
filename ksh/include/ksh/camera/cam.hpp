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

		bool empty() const
		{
			return
				zoom.empty() &&
				shiftX.empty() &&
				rotationX.empty() &&
				rotationZ.empty() &&
				rotationZLane.empty() &&
				rotationZJdgLine.empty() &&
				centerSplit.empty();
		}
	};

	template <typename G> // G: Graph or RelGraph
	void to_json(nlohmann::json& j, const CamGraphs<G>& camGraphs)
	{
		j = nlohmann::json::object();

		if (!camGraphs.zoom.empty())
		{
			j["zoom"] = camGraphs.zoom;
		}

		if (!camGraphs.shiftX.empty())
		{
			j["shift_x"] = camGraphs.shiftX;
		}

		if (!camGraphs.rotationX.empty())
		{
			j["rotation_x"] = camGraphs.rotationX;
		}

		if (!camGraphs.rotationZ.empty())
		{
			j["rotation_z"] = camGraphs.rotationZ;
		}

		if (!camGraphs.rotationZLane.empty())
		{
			j["rotation_z.lane"] = camGraphs.rotationZLane;
		}

		if (!camGraphs.rotationZJdgLine.empty())
		{
			j["rotation_z.jdgline"] = camGraphs.rotationZJdgLine;
		}

		if (!camGraphs.centerSplit.empty())
		{
			j["center_split"] = camGraphs.centerSplit;
		}
	}

	struct CamPatternParams
	{
		RelPulse length = 0;
		double scale = 1.0;
		int64_t repeat = 1;
		double repeatScale = 1.0;
		double decayOrder = 0.0;
	};

	void to_json(nlohmann::json& j, const CamPatternParams& params);

	struct CamPatternDef
	{
		CamGraphs<RelGraph> body;
		CamPatternParams params;
	};

	void to_json(nlohmann::json& j, const CamPatternDef& def);

	struct CamPatternLaserInfo
	{
		DefList<CamPatternDef> def;
		InvokeList<ByPulse<CamPatternParams>> slamInvoke;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const CamPatternLaserInfo& info);

	struct CamPatternInfo
	{
		CamPatternLaserInfo laser;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const CamPatternInfo& info);

	struct CamRoot
	{
		CamGraphs<Graph> body;
		CamPatternInfo pattern;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const CamRoot& cam);
}
