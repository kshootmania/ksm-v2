#pragma once
#include "music_game/game_defines.hpp"

struct PlaySceneArgs
{
	FilePath chartFilePath;

	MusicGame::GaugeType gaugeType = MusicGame::GaugeType::kNormalGauge;
};
