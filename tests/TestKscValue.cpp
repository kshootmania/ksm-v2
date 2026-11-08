#include <catch2/catch.hpp>
#include "HighScore/KscValue.hpp"

using namespace MusicGame;
using namespace MusicGame::Judgment;

namespace
{
	// 指定したAchievementになるプレイ結果を生成
	PlayResult MakePlayResult(Achievement achievement)
	{
		PlayResult result;
		result.score = 9000000;
		result.totalCombo = 1000;
		result.playOption.gaugeType = GaugeType::kNormalGauge;
		result.gaugePercentage = 100.0;
		result.gaugePercentageForGrade = 100.0;

		switch (achievement)
		{
		case Achievement::kPerfect:
			result.comboStats = ComboStats{ .critical = 1000, .nearFast = 0, .nearSlow = 0, .error = 0 };
			result.maxCombo = 1000;
			break;

		case Achievement::kFullCombo:
			result.comboStats = ComboStats{ .critical = 999, .nearFast = 1, .nearSlow = 0, .error = 0 };
			result.maxCombo = 1000;
			break;

		case Achievement::kCleared:
			result.comboStats = ComboStats{ .critical = 998, .nearFast = 1, .nearSlow = 0, .error = 1 };
			result.maxCombo = 999;
			break;

		default:
			break;
		}

		REQUIRE(result.achievement() == achievement);
		return result;
	}
}

TEST_CASE("KscValue applyPlayResultForCourse records only full combo or better", "[KscValue]")
{
	const KscValue orig
	{
		.score = 8000000,
		.achievement = Achievement::kNone,
		.grade = Grade::kB,
		.percent = 60,
		.maxCombo = 500,
		.playCount = 3,
		.clearCount = 0,
		.fullComboCount = 0,
		.perfectCount = 0,
	};

	// CLEAR: コースモードではフルコンボ未満はachievement・クリア回数とも更新されない
	{
		const KscValue value = orig.applyPlayResultForCourse(MakePlayResult(Achievement::kCleared));
		REQUIRE(value.score == 9000000);
		REQUIRE(value.achievement == Achievement::kNone);
		REQUIRE(value.percent == 100);
		REQUIRE(value.maxCombo == 999);
		REQUIRE(value.playCount == 4);
		REQUIRE(value.clearCount == 0);
		REQUIRE(value.fullComboCount == 0);
		REQUIRE(value.perfectCount == 0);
	}

	// FULL COMBO: achievementが更新され、クリア回数とフルコンボ回数が増える
	{
		const KscValue value = orig.applyPlayResultForCourse(MakePlayResult(Achievement::kFullCombo));
		REQUIRE(value.achievement == Achievement::kFullCombo);
		REQUIRE(value.playCount == 4);
		REQUIRE(value.clearCount == 1);
		REQUIRE(value.fullComboCount == 1);
		REQUIRE(value.perfectCount == 0);
	}

	// PERFECT: クリア回数・フルコンボ回数・パーフェクト回数が増える
	{
		const KscValue value = orig.applyPlayResultForCourse(MakePlayResult(Achievement::kPerfect));
		REQUIRE(value.achievement == Achievement::kPerfect);
		REQUIRE(value.clearCount == 1);
		REQUIRE(value.fullComboCount == 1);
		REQUIRE(value.perfectCount == 1);
	}

}
