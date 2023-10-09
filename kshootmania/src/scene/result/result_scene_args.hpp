#pragma once
#include "music_game/play_result.hpp"

struct ResultSceneArgs
{
	FilePath chartFilePath;

	kson::ChartData chartData;

	MusicGame::PlayResult playResult;
};
