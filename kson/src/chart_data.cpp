#include "kson/chart_data.hpp"

void kson::to_json(nlohmann::json& j, const ChartData& chartData)
{
	j = {
		{ "meta", chartData.meta },
		{ "beat", chartData.beat },
		{ "note", chartData.note },
		{ "audio", chartData.audio },
		{ "bg", chartData.bg },
	};

	if (!chartData.gauge.empty())
	{
		j["gauge"] = chartData.gauge;
	}

	if (!chartData.camera.empty())
	{
		j["camera"] = chartData.camera;
	}

	if (!chartData.compat.empty())
	{
		j["compat"] = chartData.compat;
	}

	if (!chartData.editor.empty())
	{
		j["editor"] = chartData.editor;
	}

	if (!chartData.impl.empty())
	{
		j["impl"] = chartData.impl;
	}
}
