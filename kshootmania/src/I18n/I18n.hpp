#pragma once
#include "Common/EnumUtils.hpp"

namespace I18n
{
	/// @brief 言語ファイルの格納ディレクトリパスを取得
	/// @return 言語ファイルの格納ディレクトリパス
	[[nodiscard]]
	FilePath GetDirectoryPath();

	/// @brief 利用可能な言語リストを取得
	/// @return 利用可能な言語リスト
	[[nodiscard]]
	Array<String> GetAvailableLanguageList();

	/// @brief 標準搭載している言語
	/// @remark ユーザー側で独自に追加したものが選択されている場合はUnknownとなる
	enum class StandardLanguage
	{
		Unknown,
		English,
		Japanese,
		Korean,
		SimplifiedChinese,
		TraditionalChinese,
	};

	/// @brief 現在の言語を取得
	/// @return 現在の言語
	[[nodiscard]]
	StandardLanguage CurrentLanguage();

	/// @brief 言語ファイルを読み込む
	void LoadLanguage(StringView name, StringView fallback = U"English");

	/// @brief カテゴリ名
	enum class Category
	{
		Select,
		Play,
		Option,
	};

	/// @brief 選曲画面テキストのキー名
	enum class Select
	{
		Judgment,
		JudgmentBT,
		JudgmentFX,
		JudgmentLaser,
		JudgmentModeOn,
		JudgmentModeOff,
		JudgmentModeAuto,
		JudgmentModeHide,
		EffRate,
		EffRateEasy,
		EffRateNormal,
		EffRateHard,
		Turn,
		TurnNormal,
		TurnMirror,
		TurnRandom,
		PlaybackSpeed,
		AssistTick,
		AssistTickOff,
		AssistTickOn,
		AutoSync,
		AutoSyncOnHigh,
		AutoSyncOnMid,
		AutoSyncOnLow,
		AutoSyncOff,
		FastSlow,
		FastSlowHide,
		FastSlowShow,
		NoteSkin,
		NoteSkinDefault,
		NoteSkinNote,
		Movie,
		MovieOff,
		MovieOn,
		Hispeed,
		PlayStatsPlayCount,
		PlayStatsClear,
		PlayStatsFullCombo,
		PlayStatsPerfect,
	};

	/// @brief プレイ画面テキストのキー名
	enum class Play
	{
		ErrorChartFileNotFound,
		ErrorSomeChartMissingInCourse,
	};

	/// @brief OPTION画面テキストのキー名
	enum class Option
	{
		FullscreenOff,
		FullscreenOn,
		TextureSizeLarge,
		TextureSizeMedium,
		TextureSizeSmall,
		AlwaysShowOtherFoldersOff,
		AlwaysShowOtherFoldersOn,
		HideAllFolderOff,
		HideAllFolderOn,
		BGDisplayModeHide,
		BGDisplayModeShowNoLayer,
		BGDisplayModeShowLayer,
		MasterVolumePercent,
		VsyncOffWithFps,
		VsyncOn,
		JudgmentOn,
		JudgmentOff,
		JudgmentAuto,
		JudgmentHide,
		LaserInputTypeKeyboard,
		LaserInputTypeMouseXY,
		LaserInputTypeSlider,
		LaserInputTypeAnalogStickXY,
		AssistTickOff,
		AssistTickOn,
		DisableIMEOff,
		DisableIMEOnLow,
		DisableIMEOnMid,
		DisableIMEOnHigh,
		TimingAdjustSuffixNoAdjustment,
		TimingAdjustSuffixLater,
		TimingAdjustSuffixEarlier,
		TimingAdjustMs,
		LaserTimingAdjustSuffixNoAdjustment,
		LaserTimingAdjustSuffixLater,
		LaserTimingAdjustSuffixEarlier,
		VisualOffsetSuffixNoAdjustment,
		VisualOffsetSuffixLater,
		VisualOffsetSuffixEarlier,
		LaserMouseDirectionLeftThenRight,
		LaserMouseDirectionRightThenRight,
		LaserMouseDirectionUpThenRight,
		LaserMouseDirectionDownThenRight,
		Off,
		On,
		SelectCloseFolderKeyBackspace,
		SelectCloseFolderKeyEsc,
		HispeedTypeHide,
		HispeedTypeShow,
		HideMouseCursorOff,
		HideMouseCursorOn,
		UseNumpadAsArrowKeysOff,
		UseNumpadAsArrowKeysOnKeyboard,
		UseNumpadAsArrowKeysOnController,
		KeyConfigNoAssign,
		KeyConfigGamepadButtonFormat,
		KeyConfigStart,
		KeyConfigBack,
		KeyConfigCategoryKeyboard,
		KeyConfigCategoryGamepad,
		GuideTop,
		GuideOption,
		GuideKeyConfig,
		ItemScreenMode,
		ItemScreenResolutionWindow,
		ItemScreenResolutionFullscreen,
		ItemDisplayLanguage,
		ItemGameplayBG,
		ItemAlwaysShowOtherFolders,
		ItemHideAllFolder,
		ItemMasterVolume,
		ItemVsync,
		ItemDefaultBTJudgmentMode,
		ItemDefaultFXJudgmentMode,
		ItemDefaultLaserJudgmentMode,
		ItemLaserInputType,
		ItemAssistTick,
		ItemAutoPlaySE,
		ItemDisableIME,
		ItemTimingAdjustment,
		ItemLaserTimingAdjustment,
		ItemVisualOffset,
		ItemMouseXInputDirection,
		ItemMouseYInputDirection,
		ItemSliderMouseInputSensitivity,
		ItemSwapLRLaser,
		ItemCloseDirectoryKey,
		Item3BTKeysStartBack,
		ItemHispeedTypeXMod,
		ItemHispeedTypeOMod,
		ItemHispeedTypeCMod,
		ItemHideMouseCursor,
		ItemUseNumpadAsArrowKeys,
	};

	/// @brief カテゴリ名とキー名からテキストを直接取得
	/// @param category カテゴリ名
	/// @param key キー名
	/// @return テキスト
	/// @remark 内部実装用。こちらではなくGetを利用すること
	[[nodiscard]]
	StringView GetByCategoryAndKey(const String& category, const String& key);

	/// @brief テキストを取得
	/// @tparam T I18nのenum型
	/// @param key キー
	/// @return テキスト
	template <typename T>
	[[nodiscard]]
	StringView Get(T key)
	{
		if constexpr (std::is_same_v<T, Select>)
		{
			return GetByCategoryAndKey(EnumUtils::EnumToString(Category::Select), EnumUtils::EnumToString(key));
		}
		else if constexpr (std::is_same_v<T, Play>)
		{
			return GetByCategoryAndKey(EnumUtils::EnumToString(Category::Play), EnumUtils::EnumToString(key));
		}
		else if constexpr (std::is_same_v<T, Option>)
		{
			return GetByCategoryAndKey(EnumUtils::EnumToString(Category::Option), EnumUtils::EnumToString(key));
		}
		else
		{
			static_assert([] { return false; }(), "I18n::Get(): Unsupported enum type.");
			return U"";
		}
	}

	/// @brief フォーマット文字列からテキストを取得
	/// @tparam T I18nのenum型
	/// @tparam ...Args フォーマット引数の型
	/// @param key キー
	/// @param ...args フォーマット引数
	/// @return テキスト
	template <typename T, typename... Args>
	[[nodiscard]]
	String Get(T key, const Args&... args)
	{
		try
		{
			return Fmt(Get(key))(args...);
		}
		catch (const fmt::format_error&)
		{
			return U"[Format Error]";
		}
	}
}
