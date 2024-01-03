#pragma once
#include "music_game/game_defines.hpp"
#include "judgment/combo_status.hpp"

namespace MusicGame
{
	struct PlayResult
	{
		int32 score = 0;

		int32 maxCombo = 0;

		int32 totalCombo = 0;

		Judgment::ComboStats comboStats;

		PlayOption playOption;

		double gaugePercentage = 0.0;

		double gaugePercentageHard = 0.0;

		bool isAborted() const;

		Achievement achievement() const;

		Grade grade() const;

		int32 gaugePercentForHighScore() const;
	};
}
