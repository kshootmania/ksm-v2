#pragma once

enum DifficultyIdx : int32
{
	kDifficultyIdxLight = 0,
	kDifficultyIdxChallenge,
	kDifficultyIdxExtended,
	kDifficultyIdxInfinite,

	kNumDifficulties,
};

constexpr int32 kLevelMin = 1;
constexpr int32 kLevelMax = 20;
constexpr int32 kNumLevels = kLevelMax - kLevelMin + 1;

inline constexpr StringView kKSHExtension = U"ksh";
inline constexpr StringView kKSONExtension = U"kson";

constexpr double kPastTimeSec = -100000.0;

enum class Medal : int32
{
	kNoMedal = 0,
	kEasyClear,
	kEasyFullCombo,
	kEasyPerfect,
	kClear,
	kHardClear,
	kFullCombo,
	kPerfect,

	kNumMedals,
};

enum class Achievement : int32
{
	kNone = 0,
	kCleared,
	kFullCombo,
	kPerfect,
};

enum class Grade : int32
{
	kNoGrade = 0,
	kD,
	kC,
	kB,
	kA,
	kAA,
	kAAA,

	kNumGrades,
};

enum class GaugeType : int32
{
	kEasyGauge = 0,
	kNormalGauge,
	kHardGauge,

	kNumGaugeTypes,
};
constexpr int32 kNumGaugeTypes = static_cast<int32>(GaugeType::kNumGaugeTypes);

enum class TurnMode : int32
{
	kNormal = 0,
	kMirror,
	kRandom,
};

enum class JudgmentPlayMode : int32
{
	kOn = 0,
	kOff,
	kAuto,
	kHide,
};
