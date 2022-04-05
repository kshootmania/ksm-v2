#pragma once

enum DifficultyIdx : int32
{
	kDifficultyIdxLight = 0,
	kDifficultyIdxChallenge,
	kDifficultyIdxExtended,
	kDifficultyIdxInfinite,

	kNumDifficulties,
};

constexpr int32 kLevelMax = 20;

inline constexpr StringView kKSHExtension = U"ksh";
inline constexpr StringView kKSONExtension = U"kson";
