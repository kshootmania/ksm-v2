#pragma once
#include "kson/chart_data.hpp"
#include "kson/util/timing_utils.hpp"
#include "ksmaudio/ksmaudio.hpp"

namespace MusicGame::Audio::AudioEffectUtils
{
	std::set<float> PrecalculateUpdateTriggerTiming(
		const kson::AudioEffectDef& def,
		const kson::Dict<kson::ByPulse<std::string>>& paramChange,
		std::int64_t totalMeasures,
		const kson::ChartData& chartData,
		const kson::TimingCache& timingCache);

	std::set<float> PrecalculateUpdateTriggerTiming(
		const kson::AudioEffectDef& def,
		std::int64_t totalMeasures,
		const kson::ChartData& chartData,
		const kson::TimingCache& timingCache);
}
