#include "ksh/util/timing_utils.hpp"
#include <cassert>

ksh::Pulse ksh::TimingUtils::TimeSigMeasurePulse(const TimeSig& timeSig, Pulse resolution)
{
    return resolution * static_cast<Pulse>(timeSig.numerator) / static_cast<Pulse>(timeSig.denominator);
}

ksh::TimingCache ksh::TimingUtils::CreateTimingCache(const BeatMap& beatMap)
{
    // FIXME: DO NOT USE UNFLEXIBLE C ASSERTION

    // There must be at least one tempo change
    assert(beatMap.bpmChanges.size() > 0);

    // Tempo at zero position must be set
    assert(beatMap.bpmChanges.contains(0));

    // There must be at least one time signature change
    assert(beatMap.timeSigChanges.size() > 0);

    // Time signature at zero position must be set
    assert(beatMap.timeSigChanges.contains(0));

    TimingCache cache;

    // The first tempo change should be placed at 0.0 ms
    cache.bpmChangeMs.emplace(0, 0.0);
    cache.bpmChangePulse.emplace(0.0, 0);

    // The first time signature change should be placed at 0.0 ms
    cache.timeSigChangePulse.emplace(0, 0);
    cache.timeSigChangeMeasureIdx.emplace(0, 0);

    // Calculate ms for each tempo change
    {
        Ms ms = 0.0;
        auto prevItr = beatMap.bpmChanges.cbegin();
        for (auto itr = std::next(prevItr); itr != beatMap.bpmChanges.cend(); ++itr)
        {
            ms += static_cast<Ms>(itr->first - prevItr->first) / beatMap.resolution * 60 * 1000 / prevItr->second;
            cache.bpmChangeMs[itr->first] = ms;
            cache.bpmChangePulse[ms] = itr->first;
            prevItr = itr;
        }
    }

    // Calculate measure count for each time signature change
    {
        Pulse pulse = 0;
        auto prevItr = beatMap.timeSigChanges.cbegin();
        for (auto itr = std::next(prevItr); itr != beatMap.timeSigChanges.cend(); ++itr)
        {
            pulse += (itr->first - prevItr->first) * (beatMap.resolution * 4 * prevItr->second.numerator / prevItr->second.denominator);
            cache.timeSigChangePulse[itr->first] = pulse;
            cache.timeSigChangeMeasureIdx[pulse] = itr->first;
            prevItr = itr;
        }
    }

    return cache;
}

ksh::Ms ksh::TimingUtils::PulseToMs(Pulse pulse, const BeatMap& beatMap, const TimingCache& cache)
{
    // Fetch the nearest BPM change
    const auto itr = CurrentAt(beatMap.bpmChanges, pulse);
    const Pulse nearestBPMChangePulse = itr->first;
    const double nearestBPM = itr->second;

    // Calculate ms using pulse difference from nearest tempo change
    const Ms ms = cache.bpmChangeMs.at(nearestBPMChangePulse) + static_cast<Ms>(pulse - nearestBPMChangePulse) / beatMap.resolution * 60 * 1000 / nearestBPM;

    return ms;
}

ksh::Pulse ksh::TimingUtils::MsToPulse(Ms ms, const BeatMap& beatMap, const TimingCache& cache)
{
    // Fetch the nearest tempo change
    const auto itr = CurrentAt(cache.bpmChangePulse, ms);
    const Ms nearestBPMChangeMs = itr->first;
    const Pulse nearestBPMChangePulse = itr->second;
    const double nearestBPM = beatMap.bpmChanges.at(nearestBPMChangePulse);

    // Calculate pulse using time difference from nearest tempo change
    const Pulse pulse = nearestBPMChangePulse + static_cast<Pulse>(beatMap.resolution * (ms - nearestBPMChangeMs) * nearestBPM / 60 / 1000);

    return pulse;
}

int64_t ksh::TimingUtils::PulseToMeasureIdx(Pulse pulse, const BeatMap& beatMap, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    const auto itr = CurrentAt(cache.timeSigChangeMeasureIdx, pulse);
    const Pulse nearestTimeSigChangePulse = itr->first;
    const int64_t nearestTimeSigChangeMeasureIdx = itr->second;
    const TimeSig& nearestTimeSig = beatMap.timeSigChanges.at(nearestTimeSigChangeMeasureIdx);

    // Calculate measure count using time difference from nearest time signature change
    const int64_t measureCount = nearestTimeSigChangeMeasureIdx + static_cast<int64_t>((pulse - nearestTimeSigChangePulse) / TimeSigMeasurePulse(nearestTimeSig, beatMap.resolution));

    return measureCount;
}

int64_t ksh::TimingUtils::MsToMeasureIdx(Ms ms, const BeatMap& beatMap, const TimingCache& cache)
{
    return PulseToMeasureIdx(MsToPulse(ms, beatMap, cache), beatMap, cache);
}

ksh::Pulse ksh::TimingUtils::MeasureIdxToPulse(int64_t measureIdx, const BeatMap& beatMap, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    const auto itr = CurrentAt(cache.timeSigChangePulse, measureIdx);
    const Pulse nearestTimeSigChangeMeasureIdx = itr->first;
    const int64_t nearestTimeSigChangePulse = itr->second;
    const TimeSig& nearestTimeSig = beatMap.timeSigChanges.at(nearestTimeSigChangeMeasureIdx);

    // Calculate measure using measure count difference from nearest time signature change
    const Pulse pulse = nearestTimeSigChangePulse + static_cast<Pulse>((measureIdx - nearestTimeSigChangeMeasureIdx) * TimeSigMeasurePulse(nearestTimeSig, beatMap.resolution));

    return pulse;
}

ksh::Pulse ksh::TimingUtils::MeasureValueToPulse(double measureValue, const BeatMap& beatMap, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    const int64_t measureIdx = static_cast<int64_t>(measureValue);
    const auto itr = CurrentAt(cache.timeSigChangePulse, measureIdx);
    const Pulse nearestTimeSigChangeMeasureIdx = itr->first;
    const int64_t nearestTimeSigChangePulse = itr->second;
    const TimeSig& nearestTimeSig = beatMap.timeSigChanges.at(nearestTimeSigChangeMeasureIdx);

    // Calculate measure using measure count difference from nearest time signature change
    const Pulse pulse = nearestTimeSigChangePulse + static_cast<Pulse>((measureValue - nearestTimeSigChangeMeasureIdx) * TimeSigMeasurePulse(nearestTimeSig, beatMap.resolution));

    return pulse;
}

ksh::Ms ksh::TimingUtils::MeasureIdxToMs(int64_t measureIdx, const BeatMap& beatMap, const TimingCache& cache)
{
    return PulseToMs(MeasureIdxToPulse(measureIdx, beatMap, cache), beatMap, cache);
}

ksh::Ms ksh::TimingUtils::MeasureValueToMs(double measureValue, const BeatMap& beatMap, const TimingCache& cache)
{
    return PulseToMs(MeasureValueToPulse(measureValue, beatMap, cache), beatMap, cache);
}

bool ksh::TimingUtils::IsPulseBarLine(Pulse pulse, const BeatMap& beatMap, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    const auto itr = CurrentAt(cache.timeSigChangeMeasureIdx, pulse);
    const Pulse nearestTimeSigChangePulse = itr->first;
    const int64_t nearestTimeSigChangeMeasureIdx = itr->second;
    const TimeSig& nearestTimeSig = beatMap.timeSigChanges.at(nearestTimeSigChangeMeasureIdx);

    return ((pulse - nearestTimeSigChangePulse) % TimeSigMeasurePulse(nearestTimeSig, beatMap.resolution)) == 0;
}

double ksh::TimingUtils::PulseTempo(Pulse pulse, const BeatMap& beatMap)
{
    // Fetch the nearest BPM change
    assert(!beatMap.bpmChanges.empty());
    return CurrentAt(beatMap.bpmChanges, pulse)->second;
}

const ksh::TimeSig& ksh::TimingUtils::PulseTimeSig(Pulse pulse, const BeatMap& beatMap, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    const auto itr = CurrentAt(cache.timeSigChangeMeasureIdx, pulse);
    const Pulse nearestTimeSigChangePulse = itr->first;
    const int64_t nearestTimeSigChangeMeasureIdx = itr->second;
    return beatMap.timeSigChanges.at(nearestTimeSigChangeMeasureIdx);
}
