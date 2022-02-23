#pragma once

// Multi-language support
namespace I18n
{
	constexpr FilePathView kDirectoryPath = U"lang";

	Array<String> GetAvailableLanguageList();

	void LoadLanguage(StringView name);

	enum Category : int32
	{
		kGeneral = 0,
		kSelect = 1,
		kPlay = 2,
		kOption = 3,
		kInputGate = 4,
	};

	enum class General : int32
	{
		kFont = 0,
		kFontP = 1,
		kError = 2,
		kAssetPathSuffix = 10,
	};

	enum class Select : int32
	{
		kSelectGuideTexturePath = 0,
		kJudgmentDisplaySettings = 1,
		kJudgmentDisplaySettingsBT = 2,
		kJudgmentDisplaySettingsFX = 3,
		kJudgmentDisplaySettingsLaser = 4,
		kJudgmentDisplaySettingsModeOn = 5,
		kJudgmentDisplaySettingsModeOff = 6,
		kJudgmentDisplaySettingsModeAuto = 7,
		kJudgmentDisplaySettingsModeHide = 8,
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
		kWindow = 0,
		kFullscreen = 1,
		kResolutionX = 2,
		kTextureSizeHighQuality = 3,
		kTextureSizeNormalQuality = 4,
		kTextureSizeLowQuality = 5,
		kAlwaysShowOtherFoldersOff = 6,
		kAlwaysShowOtherFoldersOn = 7,
		kHideAllFolderOff = 8,
		kHideAllFolderOn = 9,
	};
};
