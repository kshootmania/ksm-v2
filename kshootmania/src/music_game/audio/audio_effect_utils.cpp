#include "music_game/audio/audio_effect_utils.hpp"

namespace MusicGame::Audio::AudioEffectUtils
{
	namespace
	{
		const std::string kUpdatePeriodKey = "update_period";
		const std::string kUpdatePeriodDefaultRetrigger = "1/2";
		const std::string kUpdatePeriodDefaultOther = "0"; // Echoで使用

		const std::string kUpdateTriggerKey = "update_trigger";

		kson::RelPulse UpdatePeriodDy(const std::string& str)
		{
			return static_cast<kson::RelPulse>(kson::kResolution4 * ksmaudio::AudioEffect::StrToValueSet(ksmaudio::AudioEffect::Type::kLength, str).onMin);
		}

		const std::string& UpdatePeriodDefaultForAudioEffect(kson::AudioEffectType audioEffectType)
		{
			if (audioEffectType == kson::AudioEffectType::Retrigger)
			{
				return kUpdatePeriodDefaultRetrigger;
			}
			else
			{
				return kUpdatePeriodDefaultOther;
			}
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
			if (startY > endY)
			{
				assert(false && "Invalid order detected!");
				return { startY, startY };
			}
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

		std::set<float> PrecalculateUpdateTriggerTimingBeatOnly(std::int64_t totalMeasures, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
		{
			std::set<float> timingSet;
			for (std::int64_t measureIdx = 0; measureIdx < totalMeasures; ++measureIdx)
			{
				const auto [startY, endY] = MeasurePulsePair(measureIdx, chartData.beat, timingCache);
				for (kson::Pulse y = startY; startY <= y && y < endY; y += kson::kResolution)
				{
					timingSet.insert(static_cast<float>(kson::PulseToSec(y, chartData.beat, timingCache)));
				}
			}
			return timingSet;
		}

		std::set<float> PrecalculateUpdateTriggerTimingRetrigger(const kson::AudioEffectDef& def, const kson::Dict<kson::ByPulse<std::string>>& paramChange, std::int64_t totalMeasures, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
		{
			std::set<float> timingSet;

			// timingSetへトリガタイミングを追加する関数
			const auto fnInsertTiming = [&timingSet, &chartData, &timingCache](kson::Pulse y)
			{
				const float sec = static_cast<float>(kson::PulseToSec(y, chartData.beat, timingCache));
				timingSet.insert(sec);
			};

			// "update_period"のトリガタイミングをあらかじめ計算して記録
			// (ksonの仕様上、"update_period"については"Off>OnMin-OnMax"のうちOnMinとOnMaxの値(ノーツ判定による変化)は無視することが許容されているので、無視してOffの値のみを使用する)
			if (paramChange.contains(kUpdatePeriodKey) && !paramChange.at(kUpdatePeriodKey).empty())
			{
				// "update_period"の値に途中変更がある場合、それを加味して計算
				const kson::RelPulse defDy = UpdatePeriodDy(def.v.contains(kUpdatePeriodKey) ? def.v.at(kUpdatePeriodKey) : UpdatePeriodDefaultForAudioEffect(def.type));
				const auto& updatePeriodChanges = paramChange.at(kUpdatePeriodKey);
				for (std::int64_t measureIdx = 0; measureIdx < totalMeasures; ++measureIdx)
				{
					const auto [startY, endY] = MeasurePulsePair(measureIdx, chartData.beat, timingCache);
					const std::size_t count = kson::CountInRange(updatePeriodChanges, startY, endY);
					if (count == 0U) [[likely]]
					{
						const kson::RelPulse dy = ParamChangeUpdatePeriodDyAt(updatePeriodChanges, startY, defDy);
						if (dy > 0) // "update_period"が0の場合はトリガ更新しない
						{
							for (kson::Pulse y = startY; y < endY; y += dy)
							{
								fnInsertTiming(y);
							}
						}
					}
					else
					{
						// Pulse値を1ずつ巡回していてやや非効率だが、この処理が実行されるのは"update_period"の途中変更を含む小節のみなので大した問題ではない
						for (kson::Pulse ry = 0; ry < endY - startY; ++ry)
						{
							const kson::Pulse y = startY + ry;
							const kson::RelPulse dy = ParamChangeUpdatePeriodDyAt(updatePeriodChanges, y, defDy);
							if (dy > 0 && ry % dy == 0)
							{
								fnInsertTiming(y);
							}
						}
					}
				}
			}
			else
			{
				// "update_period"の値に途中変更がない場合、簡易的な計算のみでOKになる
				const kson::RelPulse defDy = UpdatePeriodDy(def.v.contains(kUpdatePeriodKey) ? def.v.at(kUpdatePeriodKey) : UpdatePeriodDefaultForAudioEffect(def.type));
				if (defDy > 0) // "update_period"が0の場合はトリガ更新しない
				{
					for (std::int64_t measureIdx = 0; measureIdx < totalMeasures; ++measureIdx)
					{
						const auto [startY, endY] = MeasurePulsePair(measureIdx, chartData.beat, timingCache);
						for (kson::Pulse y = startY; y < endY; y += defDy)
						{
							fnInsertTiming(y);
						}
					}
				}
			}

			// "update_trigger"の"Off>OnMin-OnMax"のOff・OnMin・OnMaxのすべてが"on"の場合のトリガタイミングをあらかじめ計算して記録
			// (これらのうち1つでも"off"の場合はプレイ中に"off"→"on"に変化しうるので事前計算の対象にしない)
			// 
			// 本来はOff・OnMin・OnMaxのそれぞれに対してトリガタイミングを事前計算して、現在がOff・OnMin・OnMaxのいずれの状況かをもとにそれらを使い分ける方が一貫性のある挙動となるが、
			// 実用上Off基準で考えてしまっても問題が出る場面はほとんどないので、そこまでする必要はない
			if (paramChange.contains(kUpdateTriggerKey))
			{
				for (const auto& [y, vStr] : paramChange.at(kUpdateTriggerKey))
				{
					const float offValue = ksmaudio::AudioEffect::StrToValueSet(ksmaudio::AudioEffect::Type::kSwitch, vStr).off;
					if (ksmaudio::AudioEffect::ValueAsBool(offValue))
					{
						fnInsertTiming(y);
					}
				}
			}

			return timingSet;
		}
	}

	// TODO: switch文ではなくインタフェース等を使ったほうが設計上良さそう？
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
			return PrecalculateUpdateTriggerTimingBarLineOnly(totalMeasures, chartData, timingCache);

		case kson::AudioEffectType::Phaser:
		case kson::AudioEffectType::Sidechain:
			return PrecalculateUpdateTriggerTimingBeatOnly(totalMeasures, chartData, timingCache);

		case kson::AudioEffectType::PitchShift:
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
			return PrecalculateUpdateTriggerTimingRetrigger(def, {}, totalMeasures, chartData, timingCache);

		case kson::AudioEffectType::Gate:
		case kson::AudioEffectType::Wobble:
			return PrecalculateUpdateTriggerTimingBarLineOnly(totalMeasures, chartData, timingCache);

		case kson::AudioEffectType::Phaser:
		case kson::AudioEffectType::Sidechain:
			return PrecalculateUpdateTriggerTimingBeatOnly(totalMeasures, chartData, timingCache);

		case kson::AudioEffectType::PitchShift:
		default:
			return {};
		}
	}
}
