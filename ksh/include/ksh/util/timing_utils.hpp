#pragma once
#include "ksh/common/common.hpp"
#include "ksh/beat/beat_map.hpp"

namespace ksh
{
	struct TimingCache
	{
		std::map<Pulse, Ms> bpmChangeMs;
		std::map<Ms, Pulse> bpmChangePulse;
		std::map<int64_t, Pulse> timeSigChangePulse;
		std::map<Pulse, int64_t> timeSigChangeMeasureIdx;
	};

	namespace TimingUtils
	{
		Pulse TimeSigMeasurePulse(const TimeSig& timeSig, Pulse resolution);

		TimingCache CreateTimingCache(const BeatMap& beatMap);

		Ms PulseToMs(Pulse pulse, const BeatMap& beatMap, const TimingCache& cache);

		Pulse MsToPulse(Ms ms, const BeatMap& beatMap, const TimingCache& cache);

		int64_t PulseToMeasureIdx(Pulse pulse, const BeatMap& beatMap, const TimingCache& cache);

		int64_t MsToMeasureIdx(Ms ms, const BeatMap& beatMap, const TimingCache& cache);

		Pulse MeasureIdxToPulse(int64_t measureIdx, const BeatMap& beatMap, const TimingCache& cache);

		Pulse MeasureValueToPulse(double measureValue, const BeatMap& beatMap, const TimingCache& cache);

		Ms MeasureIdxToMs(int64_t measureIdx, const BeatMap& beatMap, const TimingCache& cache);

		Ms MeasureValueToMs(double measureValue, const BeatMap& beatMap, const TimingCache& cache);

		bool IsPulseBarLine(Pulse pulse, const BeatMap& beatMap, const TimingCache& cache);

		double PulseTempo(Pulse pulse, const BeatMap& beatMap);

		const TimeSig& PulseTimeSig(Pulse pulse, const BeatMap& beatMap, const TimingCache& cache);
	};
}
