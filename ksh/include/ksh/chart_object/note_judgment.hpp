#pragma once

#include "ksh/chart_object/abstract_musical_segment.hpp"

struct NoteJudgment : public AbstractMusicalSegment
{
public:
    enum class Result
    {
        Undefined,
        Error,
        Near,
        Critical,
    };
    Result result;

    NoteJudgment(Measure length = 0)
        : AbstractMusicalSegment(length)
        , result(Result::Undefined)
    {
    }
};
