#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
    struct TimeSig
    {
        int64_t numerator = 4;
        int64_t denominator = 4;
    };

	void to_json(nlohmann::json& j, const TimeSig& timeSig);

    void from_json(const nlohmann::json& j, TimeSig& timeSig);
}
