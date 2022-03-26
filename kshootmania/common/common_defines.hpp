#pragma once

enum DifficultyIdx : int32
{
	kDifficultyIdxLight = 0,
	kDifficultyIdxChallenge,
	kDifficultyIdxExtended,
	kDifficultyIdxInfinite,

	kNumDifficulties,
};

inline constexpr StringView kKSHExtension = U"ksh";
inline constexpr StringView kKSONExtension = U"kson";
