#pragma once

// Multi-language support
namespace I18n
{
	constexpr FilePathView kDirectoryPath = U"lang";

	Array<String> GetAvailableLanguageList();

	void LoadLanguage(StringView name, StringView fallback = U"English");

	enum Category : int32
	{
		kGeneral = 0,
		kSelect = 1,
		kPlay = 2,
		kOption = 3,
		kInputGate = 4,

		kCategoryMax,
	};

	enum class General : int32
	{
		kFont = 0,
		kFontP = 1,
		kError = 2,
		kAssetPathSuffix = 10,
	};

	constexpr int32 kKeyIdxMax = 100;

	enum class Select : int32
	{
		kSelectGuideTexturePath = 0,
		kJudgment = 1,
		kJudgmentBT = 2,
		kJudgmentFX = 3,
		kJudgmentLaser = 4,
		kJudgmentModeOn = 5,
		kJudgmentModeOff = 6,
		kJudgmentModeAuto = 7,
		kJudgmentModeHide = 8,
		kEffRate = 9,
		kEffRateEasy = 10,
		kEffRateNormal = 11,
		kEffRateHard = 12,
		kTurn = 15,
		kTurnNormal = 16,
		kTurnMirror = 17,
		kTurnRandom = 18,
		kAssistTick = 30,
		kAssistTickOn = 31,
		kAutoSync = 37,
		kAutoSyncOnHigh = 38,
		kAutoSyncOnMid = 39,
		kAutoSyncOnLow = 40,
		kAutoSyncOff = 41,
		kFastSlow = 42,
		kFastSlowHide = 43,
		kFastSlowShow = 44,
		kNoteSkin = 45,
		kNoteSkinDefault = 46,
		kNoteSkinNote = 47,
		kMovie = 60,
		kMovieOff = 61,
		kMovieOn = 62,
		kHispeed = 63,
		kInfoPlayCount = 70,
		kInfoClear = 71,
		kInfoFullCombo = 72,
		kInfoPerfect = 73,
	};

	enum class Play : int32
	{
		kErrorFirstMeasureNotFound = 1,
		kErrorUnspecified = 2,
		kDummyTitleChartNotFoundInCourse = 3,
		kErrorSomeChartMissingInCourse = 4,
		kErrorLatestVersionRequiredForCourse = 5,
		kErrorChartInCourseLoadFailed = 6,
		kAutoSyncSaveConfirm1 = 50,
		kAutoSyncSaveConfirm2 = 51,
		kAutoSyncSaveConfirmYes = 52,
		kAutoSyncSaveConfirmNo = 53,
		kAutoSyncSaving = 55,
		kUnlockedTrack = 70,
	};

	enum class Option : int32
	{
		kFullscreenOff = 0,
		kFullscreenOn = 1,
		kResolutionX = 2,
		kTextureSizeLarge = 3,
		kTextureSizeMedium = 4,
		kTextureSizeSmall = 5,
		kAlwaysShowOtherFoldersOff = 6,
		kAlwaysShowOtherFoldersOn = 7,
		kHideAllFolderOff = 8,
		kHideAllFolderOn = 9,
		kBGDisplayModeHide = 10,
		kBGDisplayModeShowNoAnim = 11,
		kBGDisplayModeShowAnim = 12,
		kMasterVolumePercent = 13,
		kVsyncOff = 14,
		kVsyncOn = 15,
		kJudgmentOn = 16,
		kJudgmentOff = 17,
		kJudgmentAuto = 18,
		kJudgmentHide = 19,
		kLaserInputTypeKeyboard = 20,
		kLaserInputTypeMouseXY = 22,
		kLaserInputTypeSlider = 24,
		kLaserInputTypeAnalogStickXY = 25,
		kAssistTickOff = 28,
		kAssistTickOn = 29,
		kDisableIMEOff = 30,
		kDisableIMEOnLow = 31,
		kDisableIMEOnMid = 32,
		kDisableIMEOnHigh = 33,
		kTimingAdjustSuffixNoAdjustment = 34,
		kTimingAdjustSuffixLater = 35,
		kTimingAdjustSuffixEarlier = 36,
		kTimingAdjustMs = 37,
		kLaserTimingAdjustLater = 38,
		kLaserTimingAdjustEarlier = 39,
		kLaserMouseDirectionLeftThenRight = 40,
		kLaserMouseDirectionRightThenRight = 41,
		kLaserMouseDirectionUpThenRight = 42,
		kLaserMouseDirectionDownThenRight = 43,
		kLaserSignalSensitivityZero = 44,
		kDisabled = 45,
		kEnabled = 46,
		kSelectCloseFolderKeyBackspace = 47,
		kSelectCloseFolderKeyEsc = 48,
		kHispeedTypeHide = 53,
		kHispeedTypeShow = 54,
		kHideMouseCursorOff = 55,
		kHideMouseCursorOn = 56,
		kUseNumpadAsArrowKeysOff = 57,
		kUseNumpadAsArrowKeysOnKeyboard = 58,
		kUseNumpadAsArrowKeysOnController = 59,
		kKeyConfigKeyboardNoAssign = 61,
		kKeyConfigLaserKeySeparator = 62,
		kKeyConfigGamepadNoAssign = 63,
		kKeyConfigStart = 64,
		kKeyConfigBack = 65,
		kKeyConfigAuto = 66,
		kKeyConfigGamepadHyphen = 67,
		kKeyConfigCategoryPrefixKeyboard = 70,
		kKeyConfigCategoryPrefixGamepad = 71,
		kKeyConfigCategorySuffix = 72,
		kGuideTop = 90,
		kGuideOption = 91,
		kGuideOptionWithRestartRequired = 92,
		kGuideKeyConfig = 93,
	};

	enum class InputGate : int32
	{
		kErrorServerNotFound = 0,
		k404NotFound = 1,
		kErrorFileNotFound = 2,
		kErrorConnectionDropped = 3,
		kErrorLatestVersionRequired = 4,
		kErrorSongNotFound = 5,
		kErrorInvalidFileVersion = 6,
		kErrorDuringFileDownload = 8,
		kFont = 10,
		kDownloadingChart = 11,
		kDownloadingChartSlash = 12,
		kDownloadingFile = 13,
		kProgress = 14,
		kProgressPercent = 15,
		kProgressOpenBracket = 16,
		kProgressSlash = 17,
		kProgressClosedBracket = 18,
		kB = 19,
		kKB = 20,
		kMB = 21,
		kGB = 22,
	};

	template <typename T>
	constexpr Category GetCategoryOfEnumType()
	{
		if constexpr (std::is_same_v<T, General>)
		{
			return kGeneral;
		}
		else if constexpr (std::is_same_v<T, Select>)
		{
			return kSelect;
		}
		else if constexpr (std::is_same_v<T, Play>)
		{
			return kPlay;
		}
		else if constexpr (std::is_same_v<T, Option>)
		{
			return kOption;
		}
		else if constexpr (std::is_same_v<T, InputGate>)
		{
			return kInputGate;
		}
		else
		{
			static_assert(false, "Unknown type!");
		}
	}

	template <typename T>
	StringView Get(T key)
	{
		return Get(GetCategoryOfEnumType<T>(), static_cast<int32>(key));
	}

	StringView Get(Category category, int32 keyIdx);
};
