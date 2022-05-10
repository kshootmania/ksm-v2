#pragma once
#include "kson/chart_data.hpp"

namespace kson
{
	bool SaveKSONChartData(std::ostream& stream, const ChartData& chartData);

	bool SaveKSONChartData(const std::string& filePath, const ChartData& chartData);
}
