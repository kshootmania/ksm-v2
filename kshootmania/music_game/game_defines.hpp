#pragma once

namespace MusicGame
{
	constexpr double kTimeSecBeforeStart = 3.4;
	constexpr double kTimeSecBeforeStartMovie = 4.4;

	constexpr double TimeSecBeforeStart(bool movie)
	{
		return movie ? kTimeSecBeforeStartMovie : kTimeSecBeforeStart;
	}

	const std::array<KeyConfig::Button, ksh::kNumBTLanes> kBTButtons = {
		KeyConfig::kBT_A,
		KeyConfig::kBT_B,
		KeyConfig::kBT_C,
		KeyConfig::kBT_D,
	};

	const std::array<KeyConfig::Button, ksh::kNumFXLanes> kFXButtons = {
		KeyConfig::kFX_L,
		KeyConfig::kFX_R,
	};

	constexpr ksh::Pulse kPastPulse = -100000000;

	enum GaugeType : int32
	{
		kEasyGauge = 0,
		kNormalGauge,
		kHardGauge,

		kNumGaugeTypes,
	};
}
