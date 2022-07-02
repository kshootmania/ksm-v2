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

	constexpr kson::Pulse kPastPulse = -100000000;

	enum GaugeType : int32
	{
		kEasyGauge = 0,
		kNormalGauge,
		kHardGauge,

		kNumGaugeTypes,
	};

	constexpr int32 kScoreMax = 10000000;
}
