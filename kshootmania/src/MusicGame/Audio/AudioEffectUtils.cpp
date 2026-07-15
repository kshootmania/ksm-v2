#include "MusicGame/Audio/AudioEffectUtils.hpp"

namespace MusicGame::Audio::AudioEffectUtils
{
	namespace
	{
		const std::string kUpdatePeriodKeyRetrigger = "update_period";
		const std::string kUpdatePeriodDefaultRetrigger = "1/2";
		const std::string kUpdatePeriodDefaultOther = "0"; // Echoで使用

		const std::string kUpdatePeriodKeySidechain = "period";
		const std::string kUpdatePeriodDefaultSidechain = "1/4";

		const std::string kUpdateTriggerKey = "update_trigger";

		double UpdatePeriodMeasures(const std::string& str)
		{
			return static_cast<double>(ksmaudio::AudioEffect::StrToValueSet(ksmaudio::AudioEffect::Type::kLength, str).onMin);
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

		// トリガ更新判定用のカウントを計算(このカウントが変化するタイミングでトリガ更新される)
		std::int64_t UpdateTriggerCountAt(kson::Pulse y, double updatePeriodMeasures, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
		{
			const std::int64_t measureIdx = kson::PulseToMeasureIdx(y, chartData.beat, timingCache);
			const auto [startY, endY] = MeasurePulsePair(measureIdx, chartData.beat, timingCache);
			if (updatePeriodMeasures >= 1.0)
			{
				// 1小節以上の場合、曲の先頭から数えた通しの小節位置をもとにカウント
				if (endY <= startY)
				{
					return 0;
				}
				const double measureValue = static_cast<double>(measureIdx) + static_cast<double>(y - startY) / static_cast<double>(endY - startY);
				return static_cast<std::int64_t>(measureValue / updatePeriodMeasures);
			}

			const auto dy = static_cast<kson::RelPulse>(kson::kResolution4 * updatePeriodMeasures);
			if (dy <= 0)
			{
				return 0;
			}

			// 1小節未満の場合、小節頭からの経過Pulse数をもとにカウント
			return (y - startY) / dy;
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

		// updatePeriod系パラメータ(update_period, period)のトリガタイミングを生成
		// (1小節未満の値は小節頭を基準として、1小節以上の値は曲の先頭から数えた通しの小節位置を基準として等間隔にトリガ更新させる)
		std::set<float> PrecalculateUpdateTriggerTimingByUpdatePeriodKey(
			const kson::AudioEffectDef& def,
			const kson::Dict<kson::ByPulse<std::string>>& paramChange,
			const std::string& updatePeriodKey,
			const std::string& updatePeriodDefault,
			std::int64_t totalMeasures,
			const kson::ChartData& chartData,
			const kson::TimingCache& timingCache)
		{
			std::set<float> timingSet;

			// timingSetへトリガタイミングを追加する関数
			const auto fnInsertTiming = [&timingSet, &chartData, &timingCache](kson::Pulse y)
			{
				const float sec = static_cast<float>(kson::PulseToSec(y, chartData.beat, timingCache));
				timingSet.insert(sec);
			};

			// 値の途中変更を加味した区間一覧を作成(キーは区間の開始Pulse、値はupdatePeriodの小節数)
			// (ksonの仕様上、updatePeriod系については"Off>OnMin-OnMax"のうちOffとOnMaxの値を無視してOnMinの値のみを使用することが許容されているので、ここではOnMinの値のみを使用する)
			kson::ByPulse<double> updatePeriods;
			updatePeriods.emplace(kson::Pulse{ 0 }, UpdatePeriodMeasures(def.v.contains(updatePeriodKey) ? def.v.at(updatePeriodKey) : updatePeriodDefault));
			if (paramChange.contains(updatePeriodKey))
			{
				for (const auto& [y, vStr] : paramChange.at(updatePeriodKey))
				{
					updatePeriods.insert_or_assign(y, UpdatePeriodMeasures(vStr));
				}
			}

			// トリガタイミングを事前計算
			const kson::Pulse totalEndY = kson::MeasureIdxToPulse(totalMeasures, chartData.beat, timingCache);
			for (auto itr = updatePeriods.begin(); itr != updatePeriods.end(); ++itr)
			{
				const kson::Pulse regionStartY = itr->first;
				const auto nextItr = std::next(itr);
				const kson::Pulse regionEndY = nextItr == updatePeriods.end() ? totalEndY : std::min(nextItr->first, totalEndY);
				if (regionStartY >= regionEndY)
				{
					continue;
				}

				const double updatePeriodMeasures = itr->second;
				const auto dy = static_cast<kson::RelPulse>(kson::kResolution4 * updatePeriodMeasures);
				if (updatePeriodMeasures < 1.0 && dy <= 0) // 値が0の場合はトリガ更新しない
				{
					continue;
				}

				// 値の変更点でトリガ更新のカウントが変化する場合、変更点でもトリガ更新される
				if (itr != updatePeriods.begin() &&
					UpdateTriggerCountAt(regionStartY - 1, std::prev(itr)->second, chartData, timingCache) != UpdateTriggerCountAt(regionStartY, updatePeriodMeasures, chartData, timingCache))
				{
					fnInsertTiming(regionStartY);
				}

				if (updatePeriodMeasures >= 1.0)
				{
					// 1小節以上の場合、小節ごとのリセットはせず、曲の先頭から数えた通しの小節位置を基準にトリガ更新
					for (std::int64_t i = 0; ; ++i)
					{
						const double measureValue = updatePeriodMeasures * i;
						if (measureValue >= static_cast<double>(totalMeasures))
						{
							break;
						}
						const kson::Pulse y = kson::MeasureValueToPulse(measureValue, chartData.beat, timingCache);
						if (y >= regionEndY)
						{
							break;
						}
						if (y >= regionStartY)
						{
							fnInsertTiming(y);
						}
					}
				}
				else
				{
					// 1小節未満の場合、小節頭を基準にdy間隔でトリガ更新(小節ごとにリセット)
					for (std::int64_t measureIdx = kson::PulseToMeasureIdx(regionStartY, chartData.beat, timingCache); measureIdx < totalMeasures; ++measureIdx)
					{
						const auto [startY, endY] = MeasurePulsePair(measureIdx, chartData.beat, timingCache);
						if (startY >= regionEndY)
						{
							break;
						}
						for (kson::Pulse y = startY; y < endY; y += dy)
						{
							if (y >= regionEndY)
							{
								break;
							}
							if (y >= regionStartY)
							{
								fnInsertTiming(y);
							}
						}
					}
				}
			}

			return timingSet;
		}

		std::set<float> PrecalculateUpdateTriggerTimingRetrigger(const kson::AudioEffectDef& def, const kson::Dict<kson::ByPulse<std::string>>& paramChange, std::int64_t totalMeasures, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
		{
			std::set<float> timingSet = PrecalculateUpdateTriggerTimingByUpdatePeriodKey(
				def,
				paramChange,
				kUpdatePeriodKeyRetrigger,
				UpdatePeriodDefaultForAudioEffect(def.type),
				totalMeasures,
				chartData,
				timingCache);

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
						const float sec = static_cast<float>(kson::PulseToSec(y, chartData.beat, timingCache));
						timingSet.insert(sec);
					}
				}
			}

			return timingSet;
		}

		std::set<float> PrecalculateUpdateTriggerTimingSidechain(const kson::AudioEffectDef& def, const kson::Dict<kson::ByPulse<std::string>>& paramChange, std::int64_t totalMeasures, const kson::ChartData& chartData, const kson::TimingCache& timingCache)
		{
			return PrecalculateUpdateTriggerTimingByUpdatePeriodKey(
				def,
				paramChange,
				kUpdatePeriodKeySidechain,
				kUpdatePeriodDefaultSidechain,
				totalMeasures,
				chartData,
				timingCache);
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

		case kson::AudioEffectType::Sidechain:
			return PrecalculateUpdateTriggerTimingSidechain(def, paramChange, totalMeasures, chartData, timingCache);

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

		case kson::AudioEffectType::Sidechain:
			return PrecalculateUpdateTriggerTimingSidechain(def, {}, totalMeasures, chartData, timingCache);

		default:
			return {};
		}
	}
}
