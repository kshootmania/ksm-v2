#pragma once

namespace MusicGame
{
	constexpr double kTimeSecBeforeStart = 3.4;
	constexpr double kTimeSecBeforeStartMovie = 4.4;

	constexpr double TimeSecBeforeStart(bool movie)
	{
		return movie ? kTimeSecBeforeStartMovie : kTimeSecBeforeStart;
	}

	const std::array<KeyConfig::Button, kson::kNumBTLanesSZ> kBTButtons = {
		KeyConfig::kBT_A,
		KeyConfig::kBT_B,
		KeyConfig::kBT_C,
		KeyConfig::kBT_D,
	};

	const std::array<KeyConfig::Button, kson::kNumFXLanesSZ> kFXButtons = {
		KeyConfig::kFX_L,
		KeyConfig::kFX_R,
	};

	const std::array<std::array<KeyConfig::Button, 2U>, kson::kNumFXLanesSZ> kLaserButtons = { {
		{ KeyConfig::kLeftLaserL, KeyConfig::kLeftLaserR },
		{ KeyConfig::kRightLaserL, KeyConfig::kRightLaserR },
	} };

	constexpr kson::Pulse kPastPulse = -100000000;

	constexpr int32 kScoreMax = 10000000;

	constexpr int32 kGaugeValueChip = 200;
	constexpr int32 kGaugeValueChipNear = 100;
	constexpr int32 kGaugeValueLong = 50;

	constexpr double kGaugePercentageThreshold = 70.0;
	constexpr double kGaugePercentageThresholdHardWarning = 30.0;

	constexpr double kGaugeDecreasePercentByChipError = 2.0;
	constexpr double kGaugeDecreasePercentByLongError = 0.5;

	constexpr double kTiltRadians = 10_deg;

	using IsAutoPlayYN = YesNo<struct IsAutoPlayYN_tag>;
}
