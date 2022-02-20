#pragma once

#include "ksh/chart_object/abstract_note.hpp"
#include "ksh/chart_object/lane_spin.hpp"

struct LaserNote final : public AbstractNote
{
public:
    int startX;
    int endX;
    LaneSpin laneSpin;

    LaserNote(Measure length, int startX, int endX, Measure judgmentAlignmentOffsetY = 0, bool halvesCombo = false, const LaneSpin & laneSpin = LaneSpin());

    bool isSlam() const
    {
        return length <= UNIT_MEASURE / 32;
    }


    static constexpr int X_MAX = 100;
    static int charToLaserX(unsigned char c);
};
