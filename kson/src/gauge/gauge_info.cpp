#include "kson/gauge/gauge_info.hpp"

bool kson::GaugeInfo::empty() const
{
	return total == 0.0;
}

void kson::to_json(nlohmann::json& j, const GaugeInfo& gauge)
{
	j = nlohmann::json::object();

	if (gauge.total != 0.0)
	{
		j["total"] = gauge.total;
	}
}
