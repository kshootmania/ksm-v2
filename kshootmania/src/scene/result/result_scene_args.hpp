#pragma once
#include "music_game/play_result.hpp"

struct ResultSceneArgs
{
	kson::ChartData chartData;

	MusicGame::PlayResult playResult;
};
