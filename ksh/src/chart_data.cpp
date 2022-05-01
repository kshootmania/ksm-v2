#include "ksh/chart_data.hpp"

void ksh::to_json(nlohmann::json& j, const ChartData& chartData)
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
}
