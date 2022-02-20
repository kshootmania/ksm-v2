#include "ksh/chart_object/laser_note.hpp"

LaserNote::LaserNote(Measure length, int startX, int endX, Measure judgmentAlignmentOffsetY, bool halvesCombo, const LaneSpin & laneSpin)
    : AbstractNote(length)
    , startX(startX)
    , endX(endX)
    , laneSpin(laneSpin)
{
    Measure judgmentStartY = (judgmentAlignmentOffsetY + judgmentInterval(halvesCombo) - 1) / judgmentInterval(halvesCombo) * judgmentInterval(halvesCombo) - judgmentAlignmentOffsetY;
    Measure judgmentEndY = length;

    if (length <= UNIT_MEASURE / 32 && startX != endX) // Laser slam
    {
        m_judgments.emplace(0, NoteJudgment(length));
    }
    else
    {
        Measure interval = judgmentInterval(halvesCombo);
        for (Measure y = judgmentStartY; y < judgmentEndY; y += interval)
        {
            m_judgments.emplace(y, NoteJudgment(interval));
        }
    }
}

int LaserNote::charToLaserX(unsigned char c)
{
    if (c >= '0' && c <= '9')
    {
        return (c - '0') * X_MAX / 50;
    }
    else if (c >= 'A' && c <= 'Z')
    {
        return (c - 'A' + 10) * X_MAX / 50;
    }
    else if (c >= 'a' && c <= 'o')
    {
        return (c - 'a' + 36) * X_MAX / 50;
    }
    else
    {
        return -1;
    }
}
