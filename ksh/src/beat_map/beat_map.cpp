#include "ksh/beat_map/beat_map.hpp"
#include <cassert>

BeatMap::BeatMap(const std::map<Measure, double>& tempoChanges, const std::map<int, TimeSig>& timeSigChanges)
    : m_tempoChanges(tempoChanges)
    , m_timeSigChanges(timeSigChanges)
{
    // There must be at least one tempo change
    assert(m_tempoChanges.size() > 0);

    // Tempo at zero position must be set
    assert(m_tempoChanges.count(0) > 0);

    // There must be at least one time signature change
    assert(m_timeSigChanges.size() > 0);

    // Time signature at zero position must be set
    assert(m_timeSigChanges.count(0) > 0);

    // The first tempo change should be placed at 0.0 ms
    m_tempoChangeMsCache[0] = 0.0;
    m_tempoChangeMeasureCache[0.0] = 0;

    // The first time signature change should be placed at 0.0 ms
    m_timeSigChangeMeasureCache[0] = 0;
    m_timeSigChangeMeasureCountCache[0] = 0;

    // Calculate ms for each tempo change
    {
        Ms ms = 0.0;
        for (auto itr = std::next(m_tempoChanges.cbegin()); itr != m_tempoChanges.cend(); ++itr)
        {
            ms += static_cast<Ms>(itr->first - std::prev(itr)->first) / UNIT_MEASURE * 4 * 60 * 1000 / std::prev(itr)->second;
            m_tempoChangeMsCache[itr->first] = ms;
            m_tempoChangeMeasureCache[ms] = itr->first;
        }
    }

    // Calculate measure count for each time signature change
    {
        Measure measure = 0;
        for (auto itr = std::next(m_timeSigChanges.cbegin()); itr != m_timeSigChanges.cend(); ++itr)
        {
            measure += (itr->first - std::prev(itr)->first) * (UNIT_MEASURE * std::prev(itr)->second.numerator / std::prev(itr)->second.denominator);
            m_timeSigChangeMeasureCache[itr->first] = measure;
            m_timeSigChangeMeasureCountCache[measure] = itr->first;
        }
    }
}

Ms BeatMap::measureToMs(Measure measure) const
{
    // Fetch the nearest tempo change
    auto itr = m_tempoChanges.upper_bound(measure);
    if (itr != m_tempoChanges.begin()) --itr;

    // Calculate ms using measure difference from nearest tempo change
    Ms ms = m_tempoChangeMsCache.at(itr->first) + static_cast<Ms>(measure - itr->first) / UNIT_MEASURE * 4 * 60 * 1000 / itr->second;

    return ms;
}

Measure BeatMap::msToMeasure(Ms ms) const
{
    // Fetch the nearest tempo change
    auto itr = m_tempoChangeMeasureCache.upper_bound(ms);
    if (itr != m_tempoChangeMeasureCache.begin()) --itr;

    // Calculate measure using time difference from nearest tempo change
    Measure measure = itr->second + static_cast<Measure>(UNIT_MEASURE * (ms - itr->first) * m_tempoChanges.at(itr->second) / 4 / 60 / 1000);

    return measure;
}

int BeatMap::measureToMeasureCount(Measure measure) const
{
    // Fetch the nearest time signature change
    auto itr = m_timeSigChangeMeasureCountCache.upper_bound(measure);
    if (itr != m_timeSigChangeMeasureCountCache.begin()) --itr;

    // Calculate measure count using time difference from nearest time signature change
    int measureCount = itr->second + static_cast<int>((measure - itr->first) / m_timeSigChanges.at(itr->second).measure());

    return measureCount;
}

int BeatMap::msToMeasureCount(Ms ms) const
{
    return measureToMeasureCount(msToMeasure(ms));
}

Measure BeatMap::measureCountToMeasure(int measureCount) const
{
    // Fetch the nearest time signature change
    auto itr = m_timeSigChangeMeasureCache.upper_bound(measureCount);
    if (itr != m_timeSigChangeMeasureCache.begin()) --itr;

    // Calculate measure using measure count difference from nearest time signature change
    Measure measure = itr->second + static_cast<Measure>((measureCount - itr->first) * m_timeSigChanges.at(itr->first).measure());
    return measure;
}

Measure BeatMap::measureCountToMeasure(double measureCount) const
{
    // Fetch the nearest time signature change
    auto itr = m_timeSigChangeMeasureCache.upper_bound(static_cast<int>(measureCount));
    if (itr != m_timeSigChangeMeasureCache.begin()) --itr;

    // Calculate measure using measure count difference from nearest time signature change
    Measure measure = itr->second + static_cast<Measure>((measureCount - itr->first) * m_timeSigChanges.at(itr->first).measure());
    return measure;
}

Ms BeatMap::measureCountToMs(int measureCount) const
{
    return measureToMs(measureCountToMeasure(measureCount));
}

Ms BeatMap::measureCountToMs(double measureCount) const
{
    return measureToMs(measureCountToMeasure(measureCount));
}

bool BeatMap::isBarLine(Measure measure) const
{
    // Fetch the nearest time signature change
    auto itr = m_timeSigChangeMeasureCountCache.upper_bound(measure);
    if (itr != m_timeSigChangeMeasureCountCache.begin()) --itr;

    return ((measure - itr->first) % m_timeSigChanges.at(itr->second).measure()) == 0;
}

double BeatMap::tempo(Measure measure) const
{
    // Fetch the nearest tempo change
    auto itr = m_tempoChanges.upper_bound(measure);
    if (itr != m_tempoChanges.begin()) --itr;

    return itr->second;
}

TimeSig BeatMap::timeSig(Measure measure) const
{
    // Fetch the nearest time signature change
    auto itr = m_timeSigChangeMeasureCountCache.upper_bound(measure);
    if (itr != m_timeSigChangeMeasureCountCache.begin()) --itr;

    return m_timeSigChanges.at(itr->second);
}
