#include "ksh/note_builder.hpp"

namespace ksh
{

    AbstractNoteBuilder::AbstractNoteBuilder()
        : m_preparedNotePos(0)
        , m_preparedNoteLength(0)
        , m_notePrepared(false)
    {
    }

    AbstractNoteBuilder::~AbstractNoteBuilder()
    {
    }

    void AbstractNoteBuilder::resetPreparedNote()
    {
        m_notePrepared = false;
    }

    void AbstractNoteBuilder::extendPreparedNoteLength(Measure diff)
    {
        m_preparedNoteLength += diff;
    }

    BTNoteBuilder::BTNoteBuilder(Lane<BTNote> & lane)
        : m_lane(lane)
    {
    }

    void BTNoteBuilder::addPreparedNote()
    {
        if (m_notePrepared)
        {
            m_lane.emplace(m_preparedNotePos, BTNote(m_preparedNoteLength, m_preparedNotePos, m_preparedNoteHalvesCombo));
            m_notePrepared = false;
        }
    }

    void BTNoteBuilder::prepareNote(Measure y, bool halvesCombo)
    {
        if (!m_notePrepared)
        {
            m_notePrepared = true;
            m_preparedNotePos = y;
            m_preparedNoteLength = 0;
            m_preparedNoteHalvesCombo = halvesCombo;
        }
    }

    FXNoteBuilder::FXNoteBuilder(Lane<FXNote> & lane)
        : m_lane(lane)
    {
    }

    void FXNoteBuilder::addPreparedNote()
    {
        if (m_notePrepared)
        {
            m_lane.emplace(m_preparedNotePos, FXNote(m_preparedNoteLength, m_preparedNoteAudioEffectStr, m_preparedNoteAudioEffectParamStr, m_preparedNotePos, m_preparedNoteHalvesCombo));
            m_notePrepared = false;
        }
    }

    void FXNoteBuilder::prepareNote(Measure y, bool halvesCombo, const std::string & audioEffectStr, const std::string & audioEffectParamStr, bool isEditor)
    {
        if (!m_notePrepared || (isEditor && (audioEffectStr != m_preparedNoteAudioEffectStr || audioEffectParamStr != m_preparedNoteAudioEffectParamStr)))
        {
            addPreparedNote();
            m_notePrepared = true;
            m_preparedNotePos = y;
            m_preparedNoteLength = 0;
            m_preparedNoteHalvesCombo = halvesCombo;
            m_preparedNoteAudioEffectStr = audioEffectStr;
        }
    }

    LaserNoteBuilder::LaserNoteBuilder(Lane<LaserNote> & lane)
        : m_lane(lane)
    {
    }

    void LaserNoteBuilder::addPreparedNote(int preparedNoteLaserEndX)
    {
        if (m_notePrepared)
        {
            m_lane.emplace(
                m_preparedNotePos,
                LaserNote(
                    m_preparedNoteLength,
                    m_preparedNoteLaserStartX,
                    preparedNoteLaserEndX,
                    m_preparedNotePos,
                    m_preparedNoteHalvesCombo,
                    (m_preparedNoteLength <= UNIT_MEASURE / 32) ? m_preparedLaneSpin : LaneSpin()));

            m_notePrepared = false;
        }
    }

    void LaserNoteBuilder::prepareNote(Measure y, bool halvesCombo, int laserStartX)
    {
        m_notePrepared = true;
        m_preparedNotePos = y;
        m_preparedNoteLength = 0;
        m_preparedNoteHalvesCombo = halvesCombo;
        m_preparedNoteLaserStartX = laserStartX;
        m_preparedLaneSpin = LaneSpin();
    }

    void LaserNoteBuilder::prepareLaneSpin(const LaneSpin & laneSpin)
    {
        m_preparedLaneSpin = laneSpin;
    }

}
