#pragma once

#include "ksh/chart_object/abstract_note.hpp"

struct BTNote final : public AbstractNote
{
public:
    explicit BTNote(Measure length, Measure judgmentAlignmentOffsetY = 0, bool halvesCombo = false)
        : AbstractNote(length)
    {
        if (length == 0)
        {
            // Chip BT Note
            m_judgments.emplace(0, NoteJudgment());
        }
        else if (length <= oneJudgmentThreshold(halvesCombo))
        {
            // Long BT Note (too short to have multiple judgments)
            m_judgments.emplace(0, NoteJudgment(length));
        }
        else
        {
            // Long BT Note (long enough to have multiple judgments)
            Measure interval = judgmentInterval(halvesCombo);
            Measure judgmentStartY = ((judgmentAlignmentOffsetY + interval - 1) / interval + 1) * interval - judgmentAlignmentOffsetY;
            Measure judgmentEndY = length - interval;
            for (Measure y = judgmentStartY; y < judgmentEndY; y += interval)
            {
                m_judgments.emplace(y, NoteJudgment((y > judgmentEndY - interval * 2) ? (judgmentEndY - y) : interval));
            }
        }
    }
};
