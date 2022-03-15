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
	inline void to_json(nlohmann::json& j, const CamGraphs<G>& camGraphs)
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

	inline void to_json(nlohmann::json& j, const CamPatternParams& params)
	{
		j["l"] = params.length;

		if (params.scale != 1.0)
		{
			j["scale"] = params.scale;
		}

		if (params.repeat != 1)
		{
			j["repeat"] = params.repeat;
		}

		if (params.repeatScale != 1.0)
		{
			j["repeat_scale"] = params.repeatScale;
		}

		if (params.decayOrder != 0.0)
		{
			j["decay_order"] = params.decayOrder;
		}
	}

	struct CamPatternDef
	{
		CamGraphs<RelGraph> body;
		CamPatternParams params;
	};

	inline void to_json(nlohmann::json& j, const CamPatternDef& def)
	{
		j = {
			{ "body", def.body },
			{ "v", def.params },
		};
	}

	struct CamPatternInfo
	{
		DefList<CamPatternDef> defList;
		InvokeList<ByPulse<CamPatternParams>> pulseEventList;
		InvokeList<ByNotes<CamPatternParams>> noteEventList;

		bool empty() const
		{
			return defList.empty() && pulseEventList.empty() && noteEventList.empty();
		}
	};

	inline void to_json(nlohmann::json& j, const CamPatternInfo& info)
	{
		j = nlohmann::json::object();

		if (!info.defList.empty())
		{
			j["def"] = info.defList;
		}

		if (!info.pulseEventList.empty())
		{
			j["pulse_event"] = info.pulseEventList;
		}

		if (!info.noteEventList.empty())
		{
			j["note_event"] = info.noteEventList;
		}
	}

	struct CamRoot
	{
		CamGraphs<Graph> body;
		CamGraphs<Graph> tiltAssignScale = { .rotationZ = { { 0, 10.0 } } };
		CamPatternInfo patternInfo;

		bool empty() const
		{
			return body.empty() && tiltAssignScale.empty() && patternInfo.empty();
		}
	};

	inline void to_json(nlohmann::json& j, const CamRoot& cam)
	{
		j = nlohmann::json::object();

		if (!cam.body.empty())
		{
			j["body"] = cam.body;
		}

		if (!cam.tiltAssignScale.empty())
		{
			j["tilt_assign"] = cam.tiltAssignScale;
		}

		if (!cam.patternInfo.empty())
		{
			j["pattern"] = cam.patternInfo;
		}
	}
}
