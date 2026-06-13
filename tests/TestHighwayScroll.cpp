#include <catch2/catch.hpp>
#include "MusicGame/Scroll/HighwayScroll.hpp"
#include "MusicGame/GameStatus.hpp"

using namespace MusicGame::Scroll;
using namespace MusicGame;

TEST_CASE("HighwayScroll with negative currentPulse and scroll_speed", "[HighwayScroll]")
{
	kson::ChartData chartData;
	chartData.beat.bpm[0] = 120.0;
	chartData.beat.timeSig[0] = kson::TimeSig{ 4, 4 };
	chartData.beat.scrollSpeed[0] = kson::GraphPoint{ 1.0 };

	auto timingCache = kson::CreateTimingCache(chartData.beat);

	HighwayScroll highwayScroll(chartData);

	HispeedSetting hispeedSetting;
	hispeedSetting.type = HispeedType::XMod;
	hispeedSetting.value = 10;

	highwayScroll.update(hispeedSetting, 120.0);

	GameStatus gameStatus;
	gameStatus.currentPulse = -2271;
	gameStatus.currentPulseDouble = -2271.0;
	gameStatus.currentTimeSec = kson::PulseToSec(-2271, chartData.beat, timingCache);

	const int32 posY_m960 = highwayScroll.getPositionY(-960, chartData.beat, timingCache, gameStatus);
	const int32 posY_0 = highwayScroll.getPositionY(0, chartData.beat, timingCache, gameStatus);
	const int32 posY_960 = highwayScroll.getPositionY(960, chartData.beat, timingCache, gameStatus);

	REQUIRE(posY_m960 != posY_0);
	REQUIRE(posY_m960 > posY_0);
	REQUIRE(posY_0 > posY_960);
	REQUIRE(posY_m960 - posY_0 == posY_0 - posY_960);
}

TEST_CASE("HighwayScroll at currentPulse=0 with scroll_speed", "[HighwayScroll]")
{
	kson::ChartData chartData;
	chartData.beat.bpm[0] = 120.0;
	chartData.beat.timeSig[0] = kson::TimeSig{ 4, 4 };
	chartData.beat.scrollSpeed[0] = kson::GraphPoint{ 1.0 };

	auto timingCache = kson::CreateTimingCache(chartData.beat);

	HighwayScroll highwayScroll(chartData);

	HispeedSetting hispeedSetting;
	hispeedSetting.type = HispeedType::XMod;
	hispeedSetting.value = 10;

	highwayScroll.update(hispeedSetting, 120.0);

	GameStatus gameStatus;
	gameStatus.currentPulse = 0;
	gameStatus.currentPulseDouble = 0.0;
	gameStatus.currentTimeSec = 0.0;

	const int32 posY_m960 = highwayScroll.getPositionY(-960, chartData.beat, timingCache, gameStatus);
	const int32 posY_0 = highwayScroll.getPositionY(0, chartData.beat, timingCache, gameStatus);
	const int32 posY_960 = highwayScroll.getPositionY(960, chartData.beat, timingCache, gameStatus);

	REQUIRE(posY_m960 != posY_0);
	REQUIRE(posY_m960 > posY_0);
	REQUIRE(posY_0 > posY_960);
	REQUIRE(posY_m960 - posY_0 == posY_0 - posY_960);
}

TEST_CASE("HighwayScroll integrates partial scroll_speed interpolation across zero", "[HighwayScroll]")
{
	kson::ChartData chartData;
	chartData.beat.bpm[0] = 120.0;
	chartData.beat.timeSig[0] = kson::TimeSig{ 4, 4 };
	chartData.beat.scrollSpeed[0] = kson::GraphPoint{ -1.0 };
	chartData.beat.scrollSpeed[960] = kson::GraphPoint{ 1.0 };

	auto timingCache = kson::CreateTimingCache(chartData.beat);

	HighwayScroll highwayScroll(chartData);

	HispeedSetting hispeedSetting;
	hispeedSetting.type = HispeedType::XMod;
	hispeedSetting.value = 10;

	highwayScroll.update(hispeedSetting, 120.0);

	GameStatus gameStatus;
	gameStatus.currentPulse = 480;
	gameStatus.currentPulseDouble = 480.0;
	gameStatus.currentTimeSec = kson::PulseToSec(480, chartData.beat, timingCache);

	const int32 posY_240 = highwayScroll.getPositionY(240, chartData.beat, timingCache, gameStatus);
	const int32 posY_480 = highwayScroll.getPositionY(480, chartData.beat, timingCache, gameStatus);
	const int32 posY_720 = highwayScroll.getPositionY(720, chartData.beat, timingCache, gameStatus);

	REQUIRE(posY_240 > posY_480);
	REQUIRE(posY_480 > posY_720);
	REQUIRE(posY_240 - posY_480 == posY_480 - posY_720);
	REQUIRE(posY_240 - posY_480 == 33);
}
