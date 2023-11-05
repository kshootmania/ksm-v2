#pragma once
#include "music_game/game_defines.hpp"

struct PlaySceneArgs
{
	FilePath chartFilePath;

	GaugeType gaugeType = GaugeType::kNormalGauge;
};
