#pragma once

struct HighScoreInfo
{
	Medal medal = Medal::kNoMedal;

	Grade grade = Grade::kNoGrade;

	int32 score = 0;

	int32 percent = 0;

	int32 maxCombo = 0;

	int32 playCount = 0;

	int32 clearCount = 0;

	int32 fullComboCount = 0;

	static HighScoreInfo fromKscValue(const String& kscValue);
};
