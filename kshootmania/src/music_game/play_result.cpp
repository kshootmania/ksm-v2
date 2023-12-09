#include "play_result.hpp"

namespace MusicGame
{
	namespace
	{
		constexpr int32 kScoreFactorMax = kScoreMax * 9 / 10;
		constexpr int32 kGaugeFactorMax = kScoreMax - kScoreFactorMax;
	}

	bool PlayResult::isAborted() const
	{
		if (comboStats.critical + comboStats.near() + comboStats.error < totalCombo)
		{
			// 途中でプレイをやめた場合
			return true;
		}
		else
		{
			// 途中でプレイをやめていなければ判定内訳の合計がtotalComboになるはず
			assert(comboStats.critical + comboStats.near() + comboStats.error == totalCombo);
			return false;
		}
	}

	Achievement PlayResult::achievement() const
	{
		// TODO(alphaまで): EASY/HARDゲージ
		if (gaugeType != GaugeType::kNormalGauge)
		{
			throw Error(U"Not implemented");
		}

		if (isAborted())
		{
			// 途中でプレイをやめた場合
			return Achievement::kNone;
		}

		if (gaugePercentage < kGaugePercentageThreshold)
		{
			return Achievement::kNone; // ゲージがクリアラインを下回った場合
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
		if (isAborted())
		{
			// 途中でプレイをやめた場合はDにする(未プレイと区別するためにNoGradeにはしない)
			return Grade::kD;
		}

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

	int32 PlayResult::gaugePercentForHighScore() const
	{
		if (isAborted())
		{
			// 途中でプレイをやめた場合は0%扱いにする
			// (プレイ途中のゲージのパーセンテージがハイスコアに記録されないようにするため)
			return 0;
		}

		if (gaugeType == GaugeType::kHardGauge)
		{
			return static_cast<int32>(gaugePercentageHard);
		}
		else
		{
			return static_cast<int32>(gaugePercentage);
		}
	}
}
