#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstddef>

#include "ksh/chart.hpp"
#include "ksh/beat_map/beat_map.hpp"
#include "ksh/chart_object/bt_note.hpp"
#include "ksh/chart_object/fx_note.hpp"
#include "ksh/chart_object/laser_note.hpp"
#include "ksh/chart_object/line_graph.hpp"

namespace ksh
{

    template <class Note>
    using Lane = std::multimap<Measure, Note>;

    // Chart (header & body)
    class PlayableChart : public Chart
    {
    private:
        bool insertTempoChange(std::map<Measure, double> & tempoChanges, Measure y, const std::string & value);

    protected:
        std::unique_ptr<BeatMap> m_beatMap;
        std::vector<Lane<BTNote>> m_btLanes;
        std::vector<Lane<FXNote>> m_fxLanes;
        std::vector<Lane<LaserNote>> m_laserLanes;
        LineGraph m_zoomTop;
        LineGraph m_zoomBottom;
        LineGraph m_zoomSide;
        LineGraph m_centerSplit;
        LineGraph m_manualTilt;
        std::unordered_map<std::string, std::map<Measure, std::string>> m_positionalOptions;
        PlayableChart(std::string_view filename, bool isEditor);

    public:
        PlayableChart(std::string_view filename) : PlayableChart(filename, false) {}

        virtual ~PlayableChart() = default;

        const BeatMap & beatMap() const
        {
            return *m_beatMap;
        }

        const Lane<BTNote> & btLane(std::size_t idx) const
        {
            return m_btLanes.at(idx);
        }

        const Lane<FXNote> & fxLane(std::size_t idx) const
        {
            return m_fxLanes.at(idx);
        }

        const Lane<LaserNote> & laserLane(std::size_t idx) const
        {
            return m_laserLanes.at(idx);
        }

        const std::vector<Lane<BTNote>> & btLanes() const
        {
            return m_btLanes;
        }

        const std::vector<Lane<FXNote>> & fxLanes() const
        {
            return m_fxLanes;
        }

        const std::vector<Lane<LaserNote>> & laserLanes() const
        {
            return m_laserLanes;
        }

        const LineGraph & zoomTop() const
        {
            return m_zoomTop;
        }

        const LineGraph & zoomBottom() const
        {
            return m_zoomBottom;
        }

        const LineGraph & zoomSide() const
        {
            return m_zoomSide;
        }

        const LineGraph & centerSplit() const
        {
            return m_centerSplit;
        }

        const LineGraph & manualTilt() const
        {
            return m_manualTilt;
        }

        const std::unordered_map<std::string, std::map<Measure, std::string>> & positionalOptions() const
        {
            return m_positionalOptions;
        }

        std::size_t comboCount() const;
    };

}
