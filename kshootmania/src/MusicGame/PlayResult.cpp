#include "PlayResult.hpp"

namespace MusicGame
{
	namespace
	{
		constexpr int32 kScoreFactorMax = kScoreMax * 9 / 10;
		constexpr int32 kGaugeFactorMax = kScoreMax - kScoreFactorMax;
	}

	Achievement PlayResult::achievement() const
	{
		if (isAborted)
		{
			// 途中でプレイをやめた場合
			return Achievement::kNone;
		}

		// クリア判定
		bool cleared = false;
		const int32 gaugePercentageInt = static_cast<int32>(gaugePercentage);
		if (playOption.gaugeType == GaugeType::kHardGauge || playOption.gameMode == GameMode::kCourseMode)
		{
			// HARDゲージまたはコースモードの場合、1%以上でクリア
			cleared = gaugePercentageInt > kGaugePercentageThresholdHard;
		}
		else
		{
			// EASY/NORMALゲージの場合、70%以上でクリア
			cleared = gaugePercentageInt >= kGaugePercentageThreshold;
		}

		if (!cleared)
		{
			return Achievement::kNone;
		}

		if (comboStats.critical == totalCombo)
		{
			return Achievement::kPerfect;
		}
		else if (maxCombo == totalCombo)
		{
			assert(comboStats.critical + comboStats.totalNear() == totalCombo);
			return Achievement::kFullCombo;
		}
		else
		{
			return Achievement::kCleared;
		}
	}

	Grade PlayResult::grade() const
	{
		if (isAborted)
		{
			// 途中でプレイをやめた場合はDにする(未プレイと区別するためにNoGradeにはしない)
			return Grade::kD;
		}

		const int32 scoreFactor = static_cast<int32>(static_cast<int64>(score) * kScoreFactorMax / kScoreMax);
		const int32 gaugeFactor = kGaugeFactorMax * static_cast<int32>(gaugePercentageForGrade) / 100;
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
		if (isAborted)
		{
			// 途中でプレイをやめた場合は0%扱いにする
			// (プレイ途中のゲージのパーセンテージがハイスコアに記録されないようにするため)
			return 0;
		}

		return static_cast<int32>(gaugePercentage);
	}
}
