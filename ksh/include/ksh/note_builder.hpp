#pragma once

#include <string>
#include <cstddef>

#include "ksh/playable_chart.hpp"
#include "ksh/chart_object/bt_note.hpp"
#include "ksh/chart_object/fx_note.hpp"
#include "ksh/chart_object/laser_note.hpp"

namespace ksh
{

    // Temporary class for inserting notes to the lane
    class AbstractNoteBuilder
    {
    protected:
        Measure m_preparedNotePos;
        Measure m_preparedNoteLength;
        bool m_preparedNoteHalvesCombo;
        bool m_notePrepared;

    public:
        explicit AbstractNoteBuilder();

        void resetPreparedNote();

        void extendPreparedNoteLength(Measure diff);

        virtual ~AbstractNoteBuilder() = 0;
    };

    class BTNoteBuilder : public AbstractNoteBuilder
    {
    private:
        Lane<BTNote> & m_lane;

    public:
        explicit BTNoteBuilder(Lane<BTNote> & lane);

        void addPreparedNote();

        // Prepare a long BT note
        void prepareNote(Measure y, bool halvesCombo);
    };

    class FXNoteBuilder : public AbstractNoteBuilder
    {
    private:
        Lane<FXNote> & m_lane;

        // Only used in editor
        std::string m_preparedNoteAudioEffectStr;
        std::string m_preparedNoteAudioEffectParamStr;

    public:
        explicit FXNoteBuilder(Lane<FXNote> & lane);

        void addPreparedNote();

        // Prepare a long FX note (in editor, notes are split if audio effects are different)
        void prepareNote(Measure y, bool halvesCombo, const std::string & audioEffectStr = "", const std::string & audioEffectParamStr = "", bool isEditor = false);
    };

    class LaserNoteBuilder : public AbstractNoteBuilder
    {
    private:
        Lane<LaserNote> & m_lane;
        int m_preparedNoteLaserStartX;
        LaneSpin m_preparedLaneSpin;

    public:
        explicit LaserNoteBuilder(Lane<LaserNote> & lane);

        void addPreparedNote(int preparedNoteLaserEndX);

        // Prepare a laser note
        void prepareNote(Measure y, bool halvesCombo, int laserStartX);

        // Prepare a lane spin
        void prepareLaneSpin(const LaneSpin & laneSpin);
    };

}
