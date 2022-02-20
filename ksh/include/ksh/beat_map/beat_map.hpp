#pragma once

#include <map>
#include <cstdint>

#include "time_sig.hpp"

// Millisecond
using Ms = double;

class BeatMap
{
private:
    const std::map<Measure, double> m_tempoChanges;
    const std::map<int, TimeSig> m_timeSigChanges;
    mutable std::map<Measure, Ms> m_tempoChangeMsCache;
    mutable std::map<Ms, Measure> m_tempoChangeMeasureCache;
    mutable std::map<int, Measure> m_timeSigChangeMeasureCache;
    mutable std::map<Measure, int> m_timeSigChangeMeasureCountCache;

public:
    explicit BeatMap(double tempo) : BeatMap({ { 0, tempo } }) {}
    explicit BeatMap(const std::map<Measure, double> & tempoChanges = { { 0, 120.0 } },
        const std::map<int, TimeSig> & timeSigChanges = { { 0, TimeSig{ 4, 4 } } });
    Ms measureToMs(Measure measure) const;
    Measure msToMeasure(Ms ms) const;
    int measureToMeasureCount(Measure measure) const;
    int msToMeasureCount(Ms ms) const;
    Measure measureCountToMeasure(int measureCount) const;
    Measure measureCountToMeasure(double measureCount) const;
    Ms measureCountToMs(int measureCount) const;
    Ms measureCountToMs(double measureCount) const;
    bool isBarLine(Measure measure) const;
    double tempo(Measure measure) const;
    TimeSig timeSig(Measure measure) const;

    const std::map<Measure, double> & tempoChanges() const
    {
        return m_tempoChanges;
    }

    const std::map<int, TimeSig> & timeSigChanges() const
    {
        return m_timeSigChanges;
    }
};
