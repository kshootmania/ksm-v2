#pragma once
#include "kson/chart_data.hpp"

namespace kson
{
	MetaChartData LoadKSHMetaChartData(std::istream& stream);

	MetaChartData LoadKSHMetaChartData(const std::string& filePath);

	ChartData LoadKSHChartData(std::istream& stream);

	ChartData LoadKSHChartData(const std::string& filePath);
}
