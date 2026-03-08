#include <catch2/catch.hpp>
#include "MusicGame/PlayResult.hpp"
#include "MusicGame/Judgment/ComboStatus.hpp"

using namespace MusicGame;
using namespace MusicGame::Judgment;

TEST_CASE("Grade calculation with 100% gauge", "[PlayResult][Grade]")
{
	PlayResult result;
	result.gaugePercentageForGrade = 100.0;

	// スコア10000000
	result.score = 10000000;
	REQUIRE(result.grade() == Grade::kAAA);

	// グレードAAAの境界値テスト
	result.score = 9777778;
	REQUIRE(result.grade() == Grade::kAAA);
	result.score = 9777777;
	REQUIRE(result.grade() == Grade::kAA);

	// グレードAAの境界値テスト
	result.score = 9333334;
	REQUIRE(result.grade() == Grade::kAA);
	result.score = 9333333;
	REQUIRE(result.grade() == Grade::kA);

	// グレードAの境界値テスト
	result.score = 8777778;
	REQUIRE(result.grade() == Grade::kA);
	result.score = 8777777;
	REQUIRE(result.grade() == Grade::kB);

	// グレードBの境界値テスト
	result.score = 7777778;
	REQUIRE(result.grade() == Grade::kB);
	result.score = 7777777;
	REQUIRE(result.grade() == Grade::kC);

	// グレードCの境界値テスト
	result.score = 6666667;
	REQUIRE(result.grade() == Grade::kC);
	result.score = 6666666;
	REQUIRE(result.grade() == Grade::kD);

	// スコア0
	result.score = 0;
	REQUIRE(result.grade() == Grade::kD);
}

TEST_CASE("Grade calculation with 0% gauge", "[PlayResult][Grade]")
{
	PlayResult result;
	result.gaugePercentageForGrade = 0.0;

	// スコア10000000
	result.score = 10000000;
	REQUIRE(result.grade() == Grade::kA);

	// グレードAの境界値テスト
	result.score = 9888889;
	REQUIRE(result.grade() == Grade::kA);
	result.score = 9888888;
	REQUIRE(result.grade() == Grade::kB);

	// グレードBの境界値テスト
	result.score = 8888889;
	REQUIRE(result.grade() == Grade::kB);
	result.score = 8888888;
	REQUIRE(result.grade() == Grade::kC);

	// グレードCの境界値テスト
	result.score = 7777778;
	REQUIRE(result.grade() == Grade::kC);
	result.score = 7777777;
	REQUIRE(result.grade() == Grade::kD);

	// スコア0
	result.score = 0;
	REQUIRE(result.grade() == Grade::kD);
}

TEST_CASE("Achievement from gauge and combo for EASY/NORMAL gauge", "[PlayResult][Achievement]")
{
	PlayResult result;
	result.totalCombo = 1000;
	result.playOption.gaugeType = GaugeType::kNormalGauge;

	// ゲージが70%以上で全てCRITICAL判定ならPerfect
	result.comboStats = ComboStats
	{
		.critical = 1000,
		.nearFast = 0,
		.nearSlow = 0,
		.error = 0,
	};
	result.maxCombo = 1000;
	result.gaugePercentage = 100.0;
	REQUIRE(result.achievement() == Achievement::kPerfect);

	result.gaugePercentage = 70.0;
	REQUIRE(result.achievement() == Achievement::kPerfect);

	// ゲージが70%以上でコンボが途切れなければFullCombo
	result.comboStats = ComboStats
	{
		.critical = 999,
		.nearFast = 1,
		.nearSlow = 0,
		.error = 0,
	};
	result.maxCombo = 1000;
	result.gaugePercentage = 100.0;
	REQUIRE(result.achievement() == Achievement::kFullCombo);

	result.gaugePercentage = 70.0;
	REQUIRE(result.achievement() == Achievement::kFullCombo);

	// ゲージが70%以上ならコンボ継続に関わらずクリア
	result.comboStats = ComboStats
	{
		.critical = 998,
		.nearFast = 1,
		.nearSlow = 0,
		.error = 1,
	};
	result.maxCombo = 999;
	result.gaugePercentage = 100.0;
	REQUIRE(result.achievement() == Achievement::kCleared);

	result.gaugePercentage = 70.0;
	REQUIRE(result.achievement() == Achievement::kCleared);

	// EASY/NORMALゲージは70%以上でクリア
	result.gaugePercentage = 70.0;
	REQUIRE(result.achievement() == Achievement::kCleared);

	result.gaugePercentage = 69.999;
	REQUIRE(result.achievement() == Achievement::kNone);

	result.gaugePercentage = 69.0;
	REQUIRE(result.achievement() == Achievement::kNone);

	result.gaugePercentage = 0.0;
	REQUIRE(result.achievement() == Achievement::kNone);
}

TEST_CASE("Achievement from gauge and combo for HARD gauge", "[PlayResult][Achievement]")
{
	PlayResult result;
	result.totalCombo = 1000;
	result.playOption.gaugeType = GaugeType::kHardGauge;

	// ゲージが0%より大きく全てCRITICAL判定ならPerfect
	result.comboStats = ComboStats
	{
		.critical = 1000,
		.nearFast = 0,
		.nearSlow = 0,
		.error = 0,
	};
	result.maxCombo = 1000;
	result.gaugePercentage = 100.0;
	REQUIRE(result.achievement() == Achievement::kPerfect);

	result.gaugePercentage = 1.0;
	REQUIRE(result.achievement() == Achievement::kPerfect);

	// ゲージが0%より大きくコンボが途切れなければFullCombo
	result.comboStats = ComboStats
	{
		.critical = 999,
		.nearFast = 1,
		.nearSlow = 0,
		.error = 0,
	};
	result.maxCombo = 1000;
	result.gaugePercentage = 100.0;
	REQUIRE(result.achievement() == Achievement::kFullCombo);

	result.gaugePercentage = 1.0;
	REQUIRE(result.achievement() == Achievement::kFullCombo);

	// ゲージが0%より大きければコンボ継続に関わらずクリア
	result.comboStats = ComboStats
	{
		.critical = 998,
		.nearFast = 1,
		.nearSlow = 0,
		.error = 1,
	};
	result.maxCombo = 999;
	result.gaugePercentage = 100.0;
	REQUIRE(result.achievement() == Achievement::kCleared);

	result.gaugePercentage = 1.0;
	REQUIRE(result.achievement() == Achievement::kCleared);

	// HARDゲージは0%より大きければクリア
	result.gaugePercentage = 1.0;
	REQUIRE(result.achievement() == Achievement::kCleared);

	// 0.999%は0%台のためFAILED
	result.gaugePercentage = 0.999;
	REQUIRE(result.achievement() == Achievement::kNone);

	result.gaugePercentage = 0.0;
	REQUIRE(result.achievement() == Achievement::kNone);
}

TEST_CASE("Achievement when play is aborted", "[PlayResult][Achievement]")
{
	PlayResult result;
	result.totalCombo = 1000;
	result.isAborted = true;

	// プレイ中断の場合はゲージに関わらずNone
	result.comboStats = ComboStats
	{
		.critical = 500,
		.nearFast = 0,
		.nearSlow = 0,
		.error = 0,
	};
	result.maxCombo = 500;
	result.gaugePercentage = 100.0;
	REQUIRE(result.achievement() == Achievement::kNone);

	// プレイ中断の場合はゲージに関わらずNone
	result.comboStats = ComboStats
	{
		.critical = 100,
		.nearFast = 0,
		.nearSlow = 0,
		.error = 0,
	};
	result.maxCombo = 100;
	result.gaugePercentage = 100.0;
	REQUIRE(result.achievement() == Achievement::kNone);

	// isAbortedがfalseなら最後までプレイ
	result.isAborted = false;
	result.comboStats = ComboStats
	{
		.critical = 1000,
		.nearFast = 0,
		.nearSlow = 0,
		.error = 0,
	};
	result.maxCombo = 1000;
	result.gaugePercentage = 100.0;
	REQUIRE(result.achievement() == Achievement::kPerfect);
}
