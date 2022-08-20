#include "music_game/audio/audio_effect_utils.hpp"

namespace
{
	const std::string kUpdatePeriodKey = "update_period";
	const std::string kUpdatePeriodDefault = "1/2";

	kson::RelPulse UpdatePeriodDy(const std::string& str)
	{
		return static_cast<kson::RelPulse>(kson::kResolution4 * ksmaudio::AudioEffect::StrToValueSet(ksmaudio::AudioEffect::Type::kLength, str).onMin);
	}

	kson::RelPulse ParamChangeUpdatePeriodDyAt(const kson::ByPulse<std::string>& updatePeriodChanges, kson::Pulse y, kson::RelPulse defDy)
	{
		const auto itr = kson::ValueItrAt(updatePeriodChanges, y);
		if (itr == updatePeriodChanges.end() || itr->first > y)
		{
			return defDy;
		}
		return UpdatePeriodDy(itr->second);
	}

	std::pair<kson::Pulse, kson::Pulse> MeasurePulsePair(std::int64_t measureIdx, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache)
	{
		const kson::Pulse startY = kson::MeasureIdxToPulse(measureIdx, beatInfo, timingCache);
		const kson::Pulse endY = kson::MeasureIdxToPulse(measureIdx + 1, beatInfo, timingCache);
		return { startY, endY };
	}

	std::set<float> PrecalculateUpdateTriggerTimingBarLineOnly(std::int64_t totalMeasures, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
	{
		std::set<float> timingSet;
		for (std::int64_t measureIdx = 0; measureIdx < totalMeasures; ++measureIdx)
		{
			const float sec = static_cast<float>(kson::MeasureIdxToSec(measureIdx, chartData.beat, timingCache));
			timingSet.insert(sec);
		}
		return timingSet;
	}

	std::set<float> PrecalculateUpdateTriggerTimingRetriggerWithoutParamChange(const kson::AudioEffectDef& def, std::int64_t totalMeasures, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
	{
		const kson::RelPulse defDy = UpdatePeriodDy(def.v.contains(kUpdatePeriodKey) ? def.v.at(kUpdatePeriodKey) : kUpdatePeriodDefault);
		if (defDy <= 0)
		{
			return {};
		}

		std::set<float> timingSet;
		for (std::int64_t measureIdx = 0; measureIdx < totalMeasures; ++measureIdx)
		{
			const auto [startY, endY] = MeasurePulsePair(measureIdx, chartData.beat, timingCache);
			for (kson::Pulse y = startY; y < endY; y += defDy)
			{
				const float sec = static_cast<float>(kson::PulseToSec(y, chartData.beat, timingCache));
				timingSet.insert(sec);
			}
		}
		return timingSet;
	}

	std::set<float> PrecalculateUpdateTriggerTimingRetrigger(const kson::AudioEffectDef& def, const kson::Dict<kson::ByPulse<std::string>>& paramChange, std::int64_t totalMeasures, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
	{
		if (!paramChange.contains(kUpdatePeriodKey) || paramChange.at(kUpdatePeriodKey).empty())
		{
			return PrecalculateUpdateTriggerTimingRetriggerWithoutParamChange(def, totalMeasures, chartData, timingCache);
		}

		std::set<float> timingSet;

		// "update_period"のトリガタイミングをあらかじめ計算して記録
		const kson::RelPulse defDy = UpdatePeriodDy(def.v.contains(kUpdatePeriodKey) ? def.v.at(kUpdatePeriodKey) : kUpdatePeriodDefault);
		const auto& updatePeriodChanges = paramChange.at(kUpdatePeriodKey);
		for (std::int64_t measureIdx = 0; measureIdx < totalMeasures; ++measureIdx)
		{
			const auto [startY, endY] = MeasurePulsePair(measureIdx, chartData.beat, timingCache);
			const std::size_t count = kson::CountInRange(updatePeriodChanges, startY, endY);
			if (count == 0U) [[likely]]
			{
				const kson::RelPulse dy = ParamChangeUpdatePeriodDyAt(updatePeriodChanges, startY, defDy);
				if (dy > 0)
				{
					for (kson::Pulse y = startY; y < endY; y += dy)
					{
						const float sec = static_cast<float>(kson::PulseToSec(y, chartData.beat, timingCache));
						timingSet.insert(sec);
					}
				}
			}
			else
			{
				// Pulse値を1ずつ巡回していてやや非効率だが、この処理が実行されるのは"update_period"の値変化を含む小節のみなので大した問題ではない
				for (kson::Pulse ry = 0; ry < endY - startY; ++ry)
				{
					const kson::Pulse y = startY + ry;
					const kson::RelPulse dy = ParamChangeUpdatePeriodDyAt(updatePeriodChanges, y, defDy);
					if (ry % dy == 0)
					{
						const float sec = static_cast<float>(kson::PulseToSec(y, chartData.beat, timingCache));
						timingSet.insert(sec);
					}
				}
			}
		}

		return timingSet;
	}
}

namespace MusicGame::Audio::AudioEffectUtils
{
	std::set<float> PrecalculateUpdateTriggerTiming(
		const kson::AudioEffectDef& def,
		const kson::Dict<kson::ByPulse<std::string>>& paramChange,
		std::int64_t totalMeasures,
		const kson::ChartData& chartData,
		const kson::TimingCache& timingCache)
	{
		switch (def.type)
		{
		case kson::AudioEffectType::Retrigger:
		case kson::AudioEffectType::Echo:
			return PrecalculateUpdateTriggerTimingRetrigger(def, paramChange, totalMeasures, chartData, timingCache);

		case kson::AudioEffectType::Gate:
		case kson::AudioEffectType::Wobble:
		case kson::AudioEffectType::Sidechain:
			return PrecalculateUpdateTriggerTimingBarLineOnly(totalMeasures, chartData, timingCache);

		default:
			return {};
		}
	}

	std::set<float> PrecalculateUpdateTriggerTiming(
		const kson::AudioEffectDef& def,
		std::int64_t totalMeasures,
		const kson::ChartData& chartData,
		const kson::TimingCache& timingCache)
	{
		switch (def.type)
		{
		case kson::AudioEffectType::Retrigger:
		case kson::AudioEffectType::Echo:
			return PrecalculateUpdateTriggerTimingRetriggerWithoutParamChange(def, totalMeasures, chartData, timingCache);

		case kson::AudioEffectType::Gate:
		case kson::AudioEffectType::Wobble:
		case kson::AudioEffectType::Sidechain:
			return PrecalculateUpdateTriggerTimingBarLineOnly(totalMeasures, chartData, timingCache);

		default:
			return {};
		}
	}
}
