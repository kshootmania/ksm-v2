#include <catch2/catch.hpp>
#include "MusicGame/Audio/AudioEffectMain.hpp"

TEST_CASE("MakeLegacyLaserTailIntervals returns last segments not longer than 1/32", "[AudioEffectMain]")
{
	kson::ChartData chartData;
	chartData.compat.kshVersion = "171";

	// 末尾の区間が1/32
	chartData.note.laser[0][0].v.emplace(0, kson::GraphPoint{ kson::GraphValue{ 0.0 } });
	chartData.note.laser[0][0].v.emplace(240, kson::GraphPoint{ kson::GraphValue{ 0.5 } });
	chartData.note.laser[0][0].v.emplace(270, kson::GraphPoint{ kson::GraphValue{ 1.0 } });

	// 末尾の区間が1/32より長い
	chartData.note.laser[0][960].v.emplace(0, kson::GraphPoint{ kson::GraphValue{ 0.0 } });
	chartData.note.laser[0][960].v.emplace(31, kson::GraphPoint{ kson::GraphValue{ 1.0 } });

	// 末尾の区間の端が直角
	chartData.note.laser[1][0].v.emplace(0, kson::GraphPoint{ kson::GraphValue{ 0.0, 1.0 } });
	chartData.note.laser[1][0].v.emplace(30, kson::GraphPoint{ kson::GraphValue{ 1.0 } });
	chartData.note.laser[1][960].v.emplace(0, kson::GraphPoint{ kson::GraphValue{ 0.0 } });
	chartData.note.laser[1][960].v.emplace(30, kson::GraphPoint{ kson::GraphValue{ 1.0, 0.0 } });

	// 直角のみ
	chartData.note.laser[1][1920].v.emplace(0, kson::GraphPoint{ kson::GraphValue{ 0.0, 1.0 } });

	const auto intervals = MusicGame::Audio::MakeLegacyLaserTailIntervals(chartData);

	REQUIRE(intervals[0].size() == 1);
	REQUIRE(intervals[0].at(240).length == 30);
	REQUIRE(intervals[1].empty());
}

TEST_CASE("MakeLegacyLaserTailIntervals returns no intervals for charts not older than ver=200", "[AudioEffectMain]")
{
	kson::ChartData chartData;
	chartData.note.laser[0][0].v.emplace(0, kson::GraphPoint{ kson::GraphValue{ 0.0 } });
	chartData.note.laser[0][0].v.emplace(30, kson::GraphPoint{ kson::GraphValue{ 1.0 } });

	REQUIRE(MusicGame::Audio::MakeLegacyLaserTailIntervals(chartData)[0].empty());

	chartData.compat.kshVersion = "171";
	REQUIRE(MusicGame::Audio::MakeLegacyLaserTailIntervals(chartData)[0].size() == 1);
}
