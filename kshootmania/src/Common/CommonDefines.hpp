#pragma once

inline constexpr StringView kAppVersion = U"v2.0.0-alpha6";

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

constexpr double kDefaultBPM = 120.0;

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

	kCount,
};

enum class JudgmentPlayMode : int32
{
	kOn = 0,
	kOff,
	kAuto,
	kHide,
};

enum class AssistTickMode : int32
{
	kOff = 0,
	kOn,

	kCount,
};

// TODO: プレイ側で実装
enum class AutoSyncMode : int32
{
	kOff = 0,
	kLow,
	kMid,
	kHigh,

	kCount,
};

enum class FastSlowMode : int32
{
	kHide = 0,
	kShow,

	kCount,
};

enum class NoteSkinType : int32
{
	kDefault = 0,
	kNote,

	kCount,
};

enum class MovieMode : int32
{
	kOff = 0,
	kOn,

	kCount,
};

using Button = int32;

constexpr Button kUnspecifiedButton = -1;

// 注意: config.iniの値と対応しているので順番を変更してはいけない
enum ConfigurableButton : Button
{
	kButtonBT_A = 0,
	kButtonBT_B,
	kButtonBT_C,
	kButtonBT_D,
	kButtonFX_L,
	kButtonFX_R,
	kButtonLeftLaserL,
	kButtonLeftLaserR,
	kButtonRightLaserL,
	kButtonRightLaserR,
	kButtonFX_LR,
	kButtonStart,
	kButtonBack,
	kButtonAutoPlay,

	kConfigurableButtonEnumCount,
};

enum UnconfigurableButton : Button
{
	kButtonUp = kConfigurableButtonEnumCount,
	kButtonDown,
	kButtonLeft,
	kButtonRight,

	kButtonBackspace,

	kButtonEnumCount,
};

[[nodiscard]]
constexpr bool IsButtonBTFXLaser(Button button)
{
	switch (button)
	{
	case kButtonBT_A:
	case kButtonBT_B:
	case kButtonBT_C:
	case kButtonBT_D:
	case kButtonFX_L:
	case kButtonFX_R:
	case kButtonLeftLaserL:
	case kButtonLeftLaserR:
	case kButtonRightLaserL:
	case kButtonRightLaserR:
		return true;

	default:
		return false;
	}
}

[[nodiscard]]
constexpr bool IsButtonArrowKey(Button button)
{
	switch (button)
	{
	case kButtonUp:
	case kButtonDown:
	case kButtonLeft:
	case kButtonRight:
		return true;

	default:
		return false;
	}
}

#ifdef __APPLE__
// macOSプラットフォーム特有のキー定義
constexpr int32 kPlatformKeyCodeOffset = 0x10000;

struct PlatformKeyDef
{
	// config.iniに保存される値(0x10000 + 仮想キーコード)
	int32 code;

	// macOSの仮想キーコード
	int nativeCode;
};

// macOSの仮想キーコードは IOKit/hidsystem/ev_keymap.h で定義
inline constexpr std::array<PlatformKeyDef, 2> kPlatformKeys = {{
	{ kPlatformKeyCodeOffset + 0x66, 0x66 }, // kVK_JIS_Eisu(英数)
	{ kPlatformKeyCodeOffset + 0x68, 0x68 }, // kVK_JIS_Kana(かな)
}};
#endif
