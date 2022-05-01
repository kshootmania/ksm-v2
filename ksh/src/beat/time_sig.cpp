#include "ksh/beat/time_sig.hpp"

void ksh::to_json(nlohmann::json& j, const TimeSig& timeSig)
{
	j = {
		{ "n", timeSig.numerator },
		{ "d", timeSig.denominator },
	};
}

void ksh::from_json(const nlohmann::json& j, TimeSig& timeSig)
{
	j.at("n").get_to(timeSig.numerator);
	j.at("d").get_to(timeSig.denominator);
}
