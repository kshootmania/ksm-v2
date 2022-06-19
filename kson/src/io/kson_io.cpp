#include "kson/io/kson_io.hpp"
#include <fstream>

bool kson::SaveKSONChartData(std::ostream& stream, const ChartData& chartData)
{
	// TODO
	return true;
}

bool kson::SaveKSONChartData(const std::string& filePath, const ChartData& chartData)
{
	std::ofstream ofs(filePath);
	if (ofs.good())
	{
		return SaveKSONChartData(ofs, chartData);
	}
	else
	{
		return false;
	}
}
