#pragma once

#include <cstdint>

// Musical time (UNIT_MEASURE = one measure)
using Measure = int64_t;
constexpr Measure UNIT_MEASURE = 240 * 4;

// Time signature
struct TimeSig
{
public:
    uint32_t numerator;
    uint32_t denominator;
    Measure measure() const
    {
        return UNIT_MEASURE * static_cast<Measure>(numerator) / static_cast<Measure>(denominator);
    }
};
