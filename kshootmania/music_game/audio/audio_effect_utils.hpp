#pragma once
#include "kson/chart_data.hpp"
#include "kson/util/timing_utils.hpp"
#include "ksmaudio/ksmaudio.hpp"

namespace MusicGame::Audio::AudioEffectUtils
{
	std::set<double> PrecalculateUpdateTriggerTiming(bool isFX, const std::string& audioEffectName, std::int64_t totalMeasures, const kson::ChartData& chartData, const kson::TimingCache& timingCache);
}
