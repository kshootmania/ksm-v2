#include "ksh/gauge/gauge.hpp"

bool ksh::GaugeRoot::empty() const
{
	return total == 0.0;
}

void ksh::to_json(nlohmann::json& j, const GaugeRoot& gauge)
{
	j = nlohmann::json::object();

	if (gauge.total != 0.0)
	{
		j["total"] = gauge.total;
	}
}
