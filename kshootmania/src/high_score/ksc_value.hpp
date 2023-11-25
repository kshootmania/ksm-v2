#pragma once

struct KscValue
{
	int32 score = 0;

	Achievement achievement = Achievement::kNone;

	Grade grade = Grade::kNoGrade;

	int32 percent = 0;

	int32 maxCombo = 0;

	int32 playCount = 0;

	int32 clearCount = 0;

	int32 fullComboCount = 0;

	int32 perfectCount = 0;

	static KscValue FromString(const String& kscValue);
};
