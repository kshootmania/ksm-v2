#include "kson/gauge/gauge.hpp"

bool kson::GaugeRoot::empty() const
{
	return total == 0.0;
}

void kson::to_json(nlohmann::json& j, const GaugeRoot& gauge)
{
	j = nlohmann::json::object();

	if (gauge.total != 0.0)
	{
		j["total"] = gauge.total;
	}
}
