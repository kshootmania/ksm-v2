#pragma once

enum class SelectSceneSearchPhase
{
	kNone,
	kInput,
	kResult,
};

struct SelectSceneSearchParams
{
	SelectSceneSearchPhase phase = SelectSceneSearchPhase::kNone;

	String query;

	Optional<FilePath> cursorChartPath;
};
