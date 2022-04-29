#pragma once
#include "ksh/chart_data.hpp"

namespace ksh
{
	bool SaveKSONChartData(std::ostream& stream, const ChartData& chartData);

	bool SaveKSONChartData(const std::string& filePath, const ChartData& chartData);
}
