#pragma once
#include "music_game/game_defines.hpp"
#include "music_game/play_option.hpp"

struct PlaySceneArgs
{
	FilePath chartFilePath;

	MusicGame::PlayOption playOption;

	GaugeType gaugeType = GaugeType::kNormalGauge;
};
