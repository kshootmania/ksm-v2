#pragma once

struct HighScoreInfo
{
	enum Medal
	{
		kNoMedal = 0,
		kPlayed,
		kEasyClear,
		kEasyFullCombo,
		kEasyPerfect,
		kClear,
		kHardClear,
		kFullCombo,
		kPerfect,
	};
	Medal medal = kNoMedal;

	enum Grade
	{
		kNoGrade = 0,
		kD,
		kC,
		kB,
		kA,
		kAA,
		kAAA,
	};
	Grade grade = kNoGrade;

	int32 score = 0;

	int32 percent = 0;
};
