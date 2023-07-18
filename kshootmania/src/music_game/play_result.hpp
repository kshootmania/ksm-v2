#pragma once
#include "music_game/game_defines.hpp"

namespace MusicGame
{
	struct PlayResult
	{
		int32 score = 0;

		int32 maxCombo = 0;

		int32 criticalCount = 0;

		int32 nearFastCount = 0;

		int32 nearSlowCount = 0;

		int32 errorCount = 0;

		double gaugePercentage = 0.0;

		GaugeType gaugeType = kNormalGauge;
	};
}
