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
		const auto itr = kson::CurrentAt(updatePeriodChanges, y);
		if (itr == updatePeriodChanges.end() || itr->first > y)
		{
			return defDy;
		}
		return UpdatePeriodDy(itr->second);
	}

	std::pair<kson::Pulse, kson::Pulse> MeasurePulsePair(std::int64_t measureIdx, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache)
	{
		const kson::Pulse startY = kson::TimingUtils::MeasureIdxToPulse(measureIdx, beatInfo, timingCache);
		const kson::Pulse endY = kson::TimingUtils::MeasureIdxToPulse(measureIdx + 1, beatInfo, timingCache);
		return { startY, endY };
	}
}

namespace MusicGame::Audio::AudioEffectUtils
{
	std::set<double> PrecalculateUpdateTriggerTiming(bool isFX, const std::string& audioEffectName, std::int64_t totalMeasures, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
	{
		const auto& def = isFX ? chartData.audio.audioEffects.fx.def.at(audioEffectName) : chartData.audio.audioEffects.laser.def.at(audioEffectName);

		bool barLineOnly;
		switch (def.type)
		{
		case kson::AudioEffectType::Retrigger:
		case kson::AudioEffectType::Echo:
			barLineOnly = false;
			break;

		case kson::AudioEffectType::Gate:
		case kson::AudioEffectType::Wobble:
		case kson::AudioEffectType::Sidechain:
			barLineOnly = true;
			break;

		default:
			return {};
		}

		if (barLineOnly)
		{
			std::set<double> timingSet;
			for (std::int64_t measureIdx = 0; measureIdx < totalMeasures; ++measureIdx)
			{
				const double sec = MathUtils::MsToSec(kson::TimingUtils::MeasureIdxToMs(measureIdx, chartData.beat, timingCache));
				timingSet.insert(sec);
			}
			return timingSet;
		}

		// Note: The edge case behavior of UpdatePeriod is different from HSP version, but is intended.

		const auto& paramChangeDict = isFX ? chartData.audio.audioEffects.fx.paramChange.at(audioEffectName) : chartData.audio.audioEffects.laser.paramChange.at(audioEffectName);
		const kson::RelPulse defDy = UpdatePeriodDy(def.v.contains(kUpdatePeriodKey) ? def.v.at(kUpdatePeriodKey) : kUpdatePeriodDefault);
		if (!paramChangeDict.contains(kUpdatePeriodKey) || paramChangeDict.at(kUpdatePeriodKey).empty())
		{
			if (defDy <= 0)
			{
				return {};
			}

			std::set<double> timingSet;
			for (std::int64_t measureIdx = 0; measureIdx < totalMeasures; ++measureIdx)
			{
				const auto [startY, endY] = MeasurePulsePair(measureIdx, chartData.beat, timingCache);
				for (kson::Pulse y = startY; y < endY; y += defDy)
				{
					const double sec = MathUtils::MsToSec(kson::TimingUtils::PulseToMs(y, chartData.beat, timingCache));
					timingSet.insert(sec);
				}
			}
			return timingSet;
		}

		{
			const auto& updatePeriodChanges = paramChangeDict.at(kUpdatePeriodKey);
			std::set<double> timingSet;
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
							const double sec = MathUtils::MsToSec(kson::TimingUtils::PulseToMs(y, chartData.beat, timingCache));
							timingSet.insert(sec);
						}
					}
				}
				else
				{
					// Inefficient, but not that slow
					for (kson::Pulse ry = 0; ry < endY - startY; ++ry)
					{
						const kson::Pulse y = startY + ry;
						const kson::RelPulse dy = ParamChangeUpdatePeriodDyAt(updatePeriodChanges, y, defDy);
						if (ry % dy == 0)
						{
							const double sec = MathUtils::MsToSec(kson::TimingUtils::PulseToMs(y, chartData.beat, timingCache));
							timingSet.insert(sec);
						}
					}
				}
			}
			return timingSet;
		}
	}
}
