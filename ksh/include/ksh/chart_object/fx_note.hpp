#pragma once

#include <string>

#include "ksh/chart_object/abstract_note.hpp"

struct FXNote final : public AbstractNote
{
public:
    // Only used in editor
    std::string audioEffectStr;
    std::string audioEffectParamStr;

    explicit FXNote(Measure length, const std::string audioEffectStr = "", const std::string audioEffectParamStr = "", Measure judgmentAlignmentOffsetY = 0, bool halvesCombo = false)
        : AbstractNote(length)
        , audioEffectStr(audioEffectStr)
        , audioEffectParamStr(audioEffectParamStr)
    {
        Measure judgmentStartY = ((judgmentAlignmentOffsetY + judgmentInterval(halvesCombo) - 1) / judgmentInterval(halvesCombo) + 1) * judgmentInterval(halvesCombo) - judgmentAlignmentOffsetY;
        Measure judgmentEndY = length - judgmentInterval(halvesCombo);

        if (length == 0)
        {
            // Chip FX Note
            m_judgments.emplace(0, NoteJudgment());
        }
        else if (length <= oneJudgmentThreshold(halvesCombo))
        {
            // Long FX Note (too short to have multiple judgments)
            m_judgments.emplace(0, NoteJudgment(length));
        }
        else
        {
            // Long FX Note (long enough to have multiple judgments)
            Measure interval = judgmentInterval(halvesCombo);
            for (Measure y = judgmentStartY; y < judgmentEndY; y += interval)
            {
                m_judgments.emplace(y, NoteJudgment((y > judgmentEndY - interval * 2) ? (judgmentEndY - y) : interval));
            }
        }
    }
};
