#pragma once
#include "MusicGame/PlayResult.hpp"
#include "Course/CoursePlayState.hpp"
#include "Scenes/Select/SelectSceneSearchParams.hpp"

struct ResultSceneArgs
{
	FilePath chartFilePath;

	kson::ChartData chartData;

	MusicGame::PlayResult playResult;

	Optional<CoursePlayState> courseState;

	Optional<SelectSceneSearchParams> selectSearchParams;
};
