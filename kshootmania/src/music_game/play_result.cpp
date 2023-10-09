#include "play_result.hpp"

namespace MusicGame
{
	namespace
	{
		constexpr int32 kScoreFactorMax = kScoreMax * 9 / 10;
		constexpr int32 kGaugeFactorMax = kScoreMax - kScoreFactorMax;
	}

	Achievement PlayResult::achievement() const
	{
		// TODO: EASY/HARDゲージ
		if (gaugeType != GaugeType::kNormalGauge)
		{
			throw std::runtime_error("Not implemented");
		}

		// 途中でプレイをやめた場合
		if (comboStats.critical + comboStats.near() + comboStats.error < totalCombo)
		{
			return Achievement::kFailed;
		}

		// 途中でプレイをやめていなければ判定内訳の合計がtotalComboになるはず
		assert(comboStats.critical + comboStats.near() + comboStats.error == totalCombo);

		if (gaugePercentage < kGaugePercentageThreshold)
		{
			return Achievement::kFailed; // ゲージがクリアラインを下回った場合
		}
		else if (comboStats.critical == totalCombo)
		{
			return Achievement::kPerfect;
		}
		else if (maxCombo == totalCombo)
		{
			assert(comboStats.critical + comboStats.near() == totalCombo);
			return Achievement::kFullCombo;
		}
		else
		{
			return Achievement::kCleared;
		}
	}

	Grade PlayResult::grade() const
	{
		const int32 scoreFactor = static_cast<int32>(static_cast<int64>(score) * kScoreFactorMax / kScoreMax);
		const int32 gaugeFactor = kGaugeFactorMax * static_cast<int32>(gaugePercentage) / 100;
		const int32 gradeScore = scoreFactor + gaugeFactor;

		if (gradeScore >= 9800000)
		{
			return Grade::kAAA;
		}
		else if (gradeScore >= 9400000)
		{
			return Grade::kAA;
		}
		else if (gradeScore >= 8900000)
		{
			return Grade::kA;
		}
		else if (gradeScore >= 8000000)
		{
			return Grade::kB;
		}
		else if (gradeScore >= 7000000)
		{
			return Grade::kC;
		}
		else
		{
			return Grade::kD;
		}
	}
}
