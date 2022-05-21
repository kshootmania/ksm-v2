#pragma once
#include "kson/chart_data.hpp"
#include "kson/util/timing_utils.hpp"
#include "ksmaudio/ksmaudio.hpp"

namespace MusicGame::Audio::AudioEffectTimeline
{
	std::set<double> CreateTriggerUpdateTimingSet(bool isFX, const std::string& name, std::int64_t totalMeasures, const kson::ChartData& chartData, const kson::TimingCache& timingCache);
}
