#include "ksh/io/kson_io.hpp"

using json = nlohmann::json;

bool ksh::SaveKSONChartData(std::ostream& stream, const ChartData& chartData)
{
	const json j = chartData;
	stream << j.dump();
	// TODO: Error handling
	return true;
}
