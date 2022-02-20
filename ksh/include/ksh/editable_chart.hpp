#pragma once

#include "ksh/playable_chart.hpp"

namespace ksh
{

    // Chart (header & body)
    class EditableChart : public PlayableChart
    {
    public:
        EditableChart(std::string_view filename) : PlayableChart(filename, true) {}

        using PlayableChart::btLane;
        using PlayableChart::fxLane;
        using PlayableChart::laserLane;
        using PlayableChart::btLanes;
        using PlayableChart::fxLanes;
        using PlayableChart::laserLanes;

        Lane<BTNote> & btLane(std::size_t idx)
        {
            return m_btLanes.at(idx);
        }

        Lane<FXNote> & fxLane(std::size_t idx)
        {
            return m_fxLanes.at(idx);
        }

        Lane<LaserNote> & laserLane(std::size_t idx)
        {
            return m_laserLanes.at(idx);
        }

        std::vector<Lane<BTNote>> & btLanes()
        {
            return m_btLanes;
        }

        std::vector<Lane<FXNote>> & fxLanes()
        {
            return m_fxLanes;
        }

        std::vector<Lane<LaserNote>> & laserLanes()
        {
            return m_laserLanes;
        }
    };

}
