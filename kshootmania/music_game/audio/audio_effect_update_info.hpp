#pragma once
#include "kson/chart_data.hpp"

namespace MusicGame::Audio
{
	struct FXLaneState
	{
		Optional<kson::Pulse> currentLongNotePulse = none;
	};

	struct AudioEffectUpdateInfo
	{
		double currentTimeSec = 0.0;
		kson::Pulse currentPulse = 0;
		double currentBPM = 120.0;

		std::array<FXLaneState, kson::kNumFXLanes> fxLaneState;
	};
}
