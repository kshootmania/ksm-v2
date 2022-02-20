#pragma once

#include "ksh/beat_map/beat_map.hpp"

struct AbstractMusicalSegment
{
public:
    Measure length;
    explicit AbstractMusicalSegment(Measure length = 0) : length(length) {}
    virtual ~AbstractMusicalSegment() = default;
};
