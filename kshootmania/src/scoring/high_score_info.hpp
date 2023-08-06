#pragma once

struct HighScoreInfo
{
	Medal medal = Medal::kNoMedal;

	Grade grade = Grade::kNoGrade;

	int32 score = 0;

	int32 percent = 0;
};
