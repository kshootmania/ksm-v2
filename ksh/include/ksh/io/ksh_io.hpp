#pragma once
#include "ksh/chart_data.hpp"

namespace ksh
{
	std::unordered_map<std::u8string, std::u8string> LoadKSHMetaDataHashMap(std::istream& stream, bool* pIsUTF8 = nullptr);

	MetaChartData LoadKSHMetaChartData(std::istream& stream);

	MetaChartData LoadKSHMetaChartData(const std::string& filePath);

	ChartData LoadKSHChartData(std::istream& stream);

	ChartData LoadKSHChartData(const std::string& filePath);
}
