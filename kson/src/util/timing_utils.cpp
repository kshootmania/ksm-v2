#include "kson/util/timing_utils.hpp"
#include <cassert>

kson::Pulse kson::TimingUtils::TimeSigMeasurePulse(const TimeSig& timeSig)
{
    return kResolution4 * static_cast<Pulse>(timeSig.numerator) / static_cast<Pulse>(timeSig.denominator);
}

kson::TimingCache kson::TimingUtils::CreateTimingCache(const BeatInfo& beatInfo)
{
    // There must be at least one tempo change
    assert(beatInfo.bpm.size() > 0);

    // Tempo at zero position must be set
    assert(beatInfo.bpm.contains(0));

    // There must be at least one time signature change
    assert(beatInfo.timeSig.size() > 0);

    // Time signature at zero position must be set
    assert(beatInfo.timeSig.contains(0));

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
        auto prevItr = beatInfo.bpm.cbegin();
        for (auto itr = std::next(prevItr); itr != beatInfo.bpm.cend(); ++itr)
        {
            ms += static_cast<Ms>(itr->first - prevItr->first) / kResolution * 60 * 1000 / prevItr->second;
            cache.bpmChangeMs[itr->first] = ms;
            cache.bpmChangePulse[ms] = itr->first;
            prevItr = itr;
        }
    }

    // Calculate measure count for each time signature change
    {
        Pulse pulse = 0;
        auto prevItr = beatInfo.timeSig.cbegin();
        for (auto itr = std::next(prevItr); itr != beatInfo.timeSig.cend(); ++itr)
        {
            pulse += (itr->first - prevItr->first) * (kResolution4 * prevItr->second.numerator / prevItr->second.denominator);
            cache.timeSigChangePulse[itr->first] = pulse;
            cache.timeSigChangeMeasureIdx[pulse] = itr->first;
            prevItr = itr;
        }
    }

    return cache;
}

kson::Ms kson::TimingUtils::PulseToMs(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache)
{
    // Fetch the nearest BPM change
    const auto itr = CurrentAt(beatInfo.bpm, pulse);
    const Pulse nearestBPMChangePulse = itr->first;
    const double nearestBPM = itr->second;

    // Calculate ms using pulse difference from nearest tempo change
    const Ms ms = cache.bpmChangeMs.at(nearestBPMChangePulse) + static_cast<Ms>(pulse - nearestBPMChangePulse) / kResolution * 60 * 1000 / nearestBPM;

    return ms;
}

kson::Pulse kson::TimingUtils::MsToPulse(Ms ms, const BeatInfo& beatInfo, const TimingCache& cache)
{
    // Fetch the nearest tempo change
    const auto itr = CurrentAt(cache.bpmChangePulse, ms);
    const Ms nearestBPMChangeMs = itr->first;
    const Pulse nearestBPMChangePulse = itr->second;
    const double nearestBPM = beatInfo.bpm.at(nearestBPMChangePulse);

    // Calculate pulse using time difference from nearest tempo change
    const Pulse pulse = nearestBPMChangePulse + static_cast<Pulse>(kResolution * (ms - nearestBPMChangeMs) * nearestBPM / 60 / 1000);

    return pulse;
}

std::int64_t kson::TimingUtils::PulseToMeasureIdx(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    const auto itr = CurrentAt(cache.timeSigChangeMeasureIdx, pulse);
    const Pulse nearestTimeSigChangePulse = itr->first;
    const std::int64_t nearestTimeSigChangeMeasureIdx = itr->second;
    const TimeSig& nearestTimeSig = beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);

    // Calculate measure count using time difference from nearest time signature change
    const std::int64_t measureCount = nearestTimeSigChangeMeasureIdx + static_cast<std::int64_t>((pulse - nearestTimeSigChangePulse) / TimeSigMeasurePulse(nearestTimeSig));

    return measureCount;
}

std::int64_t kson::TimingUtils::MsToMeasureIdx(Ms ms, const BeatInfo& beatInfo, const TimingCache& cache)
{
    return PulseToMeasureIdx(MsToPulse(ms, beatInfo, cache), beatInfo, cache);
}

kson::Pulse kson::TimingUtils::MeasureIdxToPulse(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    const auto itr = CurrentAt(cache.timeSigChangePulse, measureIdx);
    const Pulse nearestTimeSigChangeMeasureIdx = itr->first;
    const std::int64_t nearestTimeSigChangePulse = itr->second;
    const TimeSig& nearestTimeSig = beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);

    // Calculate measure using measure count difference from nearest time signature change
    const Pulse pulse = nearestTimeSigChangePulse + static_cast<Pulse>((measureIdx - nearestTimeSigChangeMeasureIdx) * TimeSigMeasurePulse(nearestTimeSig));

    return pulse;
}

kson::Pulse kson::TimingUtils::MeasureValueToPulse(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    const std::int64_t measureIdx = static_cast<std::int64_t>(measureValue);
    const auto itr = CurrentAt(cache.timeSigChangePulse, measureIdx);
    const Pulse nearestTimeSigChangeMeasureIdx = itr->first;
    const std::int64_t nearestTimeSigChangePulse = itr->second;
    const TimeSig& nearestTimeSig = beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);

    // Calculate measure using measure count difference from nearest time signature change
    const Pulse pulse = nearestTimeSigChangePulse + static_cast<Pulse>((measureValue - nearestTimeSigChangeMeasureIdx) * TimeSigMeasurePulse(nearestTimeSig));

    return pulse;
}

kson::Ms kson::TimingUtils::MeasureIdxToMs(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache)
{
    return PulseToMs(MeasureIdxToPulse(measureIdx, beatInfo, cache), beatInfo, cache);
}

kson::Ms kson::TimingUtils::MeasureValueToMs(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache)
{
    return PulseToMs(MeasureValueToPulse(measureValue, beatInfo, cache), beatInfo, cache);
}

bool kson::TimingUtils::IsPulseBarLine(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    const auto itr = CurrentAt(cache.timeSigChangeMeasureIdx, pulse);
    const Pulse nearestTimeSigChangePulse = itr->first;
    const std::int64_t nearestTimeSigChangeMeasureIdx = itr->second;
    const TimeSig& nearestTimeSig = beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);

    return ((pulse - nearestTimeSigChangePulse) % TimeSigMeasurePulse(nearestTimeSig)) == 0;
}

double kson::TimingUtils::PulseTempo(Pulse pulse, const BeatInfo& beatInfo)
{
    // Fetch the nearest BPM change
    assert(!beatInfo.bpm.empty());
    return CurrentAt(beatInfo.bpm, pulse)->second;
}

const kson::TimeSig& kson::TimingUtils::PulseTimeSig(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    const auto itr = CurrentAt(cache.timeSigChangeMeasureIdx, pulse);
    const Pulse nearestTimeSigChangePulse = itr->first;
    const std::int64_t nearestTimeSigChangeMeasureIdx = itr->second;
    return beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);
}
