#include "OptionScene.hpp"
#include "OptionAssets.hpp"
#include "Common/FrameRateLimit.hpp"
#include "Common/IMEUtils.hpp"
#include "RuntimeConfig.hpp"
#include "Scenes/Title/TitleScene.hpp"
#include "Input/InputUtils.hpp"

// TODO: TextureIdxまわりどうにかする

namespace
{
	constexpr Duration kFadeDuration = 0.6s;

	constexpr int32 kHeaderX = 12;
	constexpr int32 kHeaderY = 10;
	constexpr int32 kHeaderWidth = 270;
	constexpr int32 kHeaderHeight = 24;
	constexpr int32 kOptionMenuOffsetX = 20;
	constexpr int32 kOptionMenuOffsetY = 40;
	constexpr int32 kGuideX = 20;
	constexpr int32 kGuideY = 456;

	constexpr int32 kMasterVolumeMin = 0;
	constexpr int32 kMasterVolumeMax = 100;
	constexpr int32 kMasterVolumeDefault = 100;
	constexpr int32 kTimingAdjustMin = -10000;
	constexpr int32 kTimingAdjustMax = 10000;
	constexpr int32 kTimingAdjustDefault = 0;
	constexpr int32 kLaserSignalSensitivityMin = -10000;
	constexpr int32 kLaserSignalSensitivityMax = 10000;
	constexpr int32 kLaserSignalSensitivityDefault = 50;

	std::array<OptionMenu, OptionScene::kOptionMenuTypeEnumCount> MakeOptionMenus()
	{
		using StrPair = std::pair<String, String>;
		using IntStrPair = std::pair<int32, String>;
		using DoubleStrPair = std::pair<double, String>;
		using CreateInfo = OptionMenuField::CreateInfo;

		Array<StrPair> availableLanguageStrPairs;
		for (const auto& language : I18n::GetAvailableLanguageList())
		{
			availableLanguageStrPairs.emplace_back(language, language);
		}

		return {
			OptionMenu(OptionTexture::kMenuKeyValueDisplaySound, {
				CreateInfo::Enum(ConfigIni::Key::kFullScreen, Array<StringView>{
					I18n::Get(I18n::Option::FullscreenOff),
					I18n::Get(I18n::Option::FullscreenOn),
				}),
				CreateInfo::Enum(ConfigIni::Key::kWindowResolution, Array<StrPair>{
					StrPair{ U"320,240", U"320x240" },
					StrPair{ U"480,320", U"480x320" },
					StrPair{ U"480,360", U"480x360" },
					StrPair{ U"512,384", U"512x384" },
					StrPair{ U"580,435", U"580x435" },
					StrPair{ U"640,480", U"640x480" },
					StrPair{ U"800,600", U"800x600" },
					StrPair{ U"1024,768", U"1024x768" },
					StrPair{ U"1280,720", U"1280x720" },
					StrPair{ U"1280,1024", U"1280x1024" },
					StrPair{ U"1366,768", U"1366x768" },
					StrPair{ U"1920,1080", U"1920x1080" },
					StrPair{ U"1920,1200", U"1920x1200" },
				}),
				CreateInfo::Enum(ConfigIni::Key::kFullScreenResolution, Array<StrPair>{
					StrPair{ U"640,480", U"640x480" },
					StrPair{ U"800,600", U"800x600" },
					StrPair{ U"1024,600", U"1024x600" },
					StrPair{ U"1024,768", U"1024x768" },
					StrPair{ U"1280,720", U"1280x720" },
					StrPair{ U"1280,800", U"1280x800" },
					StrPair{ U"1280,1024", U"1280x1024" },
					StrPair{ U"1366,768", U"1366x768" },
					StrPair{ U"1920,1080", U"1920x1080" },
					StrPair{ U"1920,1200", U"1920x1200" },
				}),
				CreateInfo::Enum(ConfigIni::Key::kLanguage, availableLanguageStrPairs).setOnChangeCallback(
					[]
					{
						I18n::LoadLanguage(ConfigIni::GetString(ConfigIni::Key::kLanguage));
					}),
				/*CreateInfo::Enum(ConfigIni::Key::kTextureSize, Array<IntStrPair>{
					IntStrPair{ ConfigIni::Value::TextureSize::kSmall, I18n::Get(I18n::Option::TextureSizeSmall) },
					IntStrPair{ ConfigIni::Value::TextureSize::kMedium, I18n::Get(I18n::Option::TextureSizeMedium) },
					IntStrPair{ ConfigIni::Value::TextureSize::kLarge, I18n::Get(I18n::Option::TextureSizeLarge) },
				}),*/
				CreateInfo::Enum(ConfigIni::Key::kBGDisplayMode, Array<IntStrPair>{
					IntStrPair{ ConfigIni::Value::BGDisplayMode::kHide, I18n::Get(I18n::Option::BGDisplayModeHide) },
					IntStrPair{ ConfigIni::Value::BGDisplayMode::kShowNoLayer, I18n::Get(I18n::Option::BGDisplayModeShowNoLayer) },
					IntStrPair{ ConfigIni::Value::BGDisplayMode::kShowLayer, I18n::Get(I18n::Option::BGDisplayModeShowLayer) },
				}).setKeyTextureIdx(5),
				CreateInfo::Enum(ConfigIni::Key::kAlwaysShowOtherFolders, Array<StringView>{
					I18n::Get(I18n::Option::AlwaysShowOtherFoldersOff),
					I18n::Get(I18n::Option::AlwaysShowOtherFoldersOn),
				}).setKeyTextureIdx(6),
				CreateInfo::Enum(ConfigIni::Key::kHideAllFolder, Array<StringView>{
					I18n::Get(I18n::Option::HideAllFolderOff),
					I18n::Get(I18n::Option::HideAllFolderOn),
				}).setKeyTextureIdx(7),
				CreateInfo::Int(ConfigIni::Key::kMasterVolume, kMasterVolumeMin, kMasterVolumeMax, kMasterVolumeDefault, I18n::Get(I18n::Option::MasterVolumePercent), 5)
					.setKeyTextureIdx(8)
					.setOnChangeCallback(
						[]
						{
							const int32 volume = ConfigIni::GetInt(ConfigIni::Key::kMasterVolume, kMasterVolumeDefault);
							ksmaudio::SetMasterVolume(volume / 100.0);
						}),
				CreateInfo::Enum(ConfigIni::Key::kVsync, Array<StrPair>{
					StrPair{ U"0;120", U"{}(120fps)"_fmt(I18n::Get(I18n::Option::VsyncOff)) },
					StrPair{ U"0;144", U"{}(144fps)"_fmt(I18n::Get(I18n::Option::VsyncOff)) },
					StrPair{ U"0;300", U"{}(300fps)"_fmt(I18n::Get(I18n::Option::VsyncOff)) },
					StrPair{ U"1", I18n::Get(I18n::Option::VsyncOn) },
				}).setKeyTextureIdx(9),
			}),
			OptionMenu(OptionTexture::kMenuKeyValueInputJudgment, {
				CreateInfo::Enum(ConfigIni::Key::kJudgmentModeBT, Array<StringView>{
					I18n::Get(I18n::Option::JudgmentOn),
					I18n::Get(I18n::Option::JudgmentOff),
					I18n::Get(I18n::Option::JudgmentAuto),
					I18n::Get(I18n::Option::JudgmentHide),
				}).setOnChangeCallback([]() {
					RuntimeConfig::SetJudgmentPlayModeBT(static_cast<JudgmentPlayMode>(ConfigIni::GetInt(ConfigIni::Key::kJudgmentModeBT)));
				}),
				CreateInfo::Enum(ConfigIni::Key::kJudgmentModeFX, Array<StringView>{
					I18n::Get(I18n::Option::JudgmentOn),
					I18n::Get(I18n::Option::JudgmentOff),
					I18n::Get(I18n::Option::JudgmentAuto),
					I18n::Get(I18n::Option::JudgmentHide),
				}).setOnChangeCallback([]() {
					RuntimeConfig::SetJudgmentPlayModeFX(static_cast<JudgmentPlayMode>(ConfigIni::GetInt(ConfigIni::Key::kJudgmentModeFX)));
				}),
				CreateInfo::Enum(ConfigIni::Key::kJudgmentModeLaser, Array<StringView>{
					I18n::Get(I18n::Option::JudgmentOn),
					I18n::Get(I18n::Option::JudgmentOff),
					I18n::Get(I18n::Option::JudgmentAuto),
					I18n::Get(I18n::Option::JudgmentHide),
				}).setOnChangeCallback([]() {
					RuntimeConfig::SetJudgmentPlayModeLaser(static_cast<JudgmentPlayMode>(ConfigIni::GetInt(ConfigIni::Key::kJudgmentModeLaser)));
				}),
				CreateInfo::Enum(ConfigIni::Key::kLaserInputType, Array<IntStrPair>{
					IntStrPair{ ConfigIni::Value::LaserInputType::kKeyboard, I18n::Get(I18n::Option::LaserInputTypeKeyboard) },
					IntStrPair{ ConfigIni::Value::LaserInputType::kSlider, I18n::Get(I18n::Option::LaserInputTypeSlider) },
#if !defined(__APPLE__) // macOSではマウスデバイスの取得に入力監視の許可が必要で、配布アプリケーション用に正式なコード署名が必要になる(要Apple Developer Program加入)とみられるため実装済みだが一旦除外
					IntStrPair{ ConfigIni::Value::LaserInputType::kMouseXY, I18n::Get(I18n::Option::LaserInputTypeMouseXY) },
#endif
					IntStrPair{ ConfigIni::Value::LaserInputType::kAnalogStickXY, I18n::Get(I18n::Option::LaserInputTypeAnalogStickXY) },
				}).setOnChangeCallback([]() {
					InputUtils::InitKsmaxisForCurrentLaserInput();
				}),
				CreateInfo::Enum(ConfigIni::Key::kAssistTick, Array<StringView>{
					I18n::Get(I18n::Option::AssistTickOff),
					I18n::Get(I18n::Option::AssistTickOn),
				}).setKeyTextureIdx(4),
				CreateInfo::Enum(ConfigIni::Key::kDisableIME, Array<StringView>{
					I18n::Get(I18n::Option::DisableIMEOff),
					I18n::Get(I18n::Option::DisableIMEOnLow),
					I18n::Get(I18n::Option::DisableIMEOnMid),
					I18n::Get(I18n::Option::DisableIMEOnHigh),
				}).setKeyTextureIdx(5),
				CreateInfo::Int(ConfigIni::Key::kInputDelay, kTimingAdjustMin, kTimingAdjustMax, kTimingAdjustDefault, I18n::Get(I18n::Option::TimingAdjustMs))
				.setAdditionalSuffixes(
					I18n::Get(I18n::Option::TimingAdjustSuffixNoAdjustment),
					I18n::Get(I18n::Option::TimingAdjustSuffixLater),
					I18n::Get(I18n::Option::TimingAdjustSuffixEarlier))
				.setKeyTextureIdx(6),
				CreateInfo::Int(ConfigIni::Key::kLaserInputDelay, kTimingAdjustMin, kTimingAdjustMax, kTimingAdjustDefault, I18n::Get(I18n::Option::TimingAdjustMs))
				.setAdditionalSuffixes(
					I18n::Get(I18n::Option::TimingAdjustSuffixNoAdjustment),
					I18n::Get(I18n::Option::LaserTimingAdjustLater),
					I18n::Get(I18n::Option::LaserTimingAdjustEarlier))
				.setKeyTextureIdx(7),
				CreateInfo::Enum(ConfigIni::Key::kLaserMouseDirectionX, Array<StringView>{
					I18n::Get(I18n::Option::LaserMouseDirectionLeftThenRight),
					I18n::Get(I18n::Option::LaserMouseDirectionRightThenRight),
				}).setKeyTextureIdx(8),
				CreateInfo::Enum(ConfigIni::Key::kLaserMouseDirectionY, Array<StringView>{
					I18n::Get(I18n::Option::LaserMouseDirectionUpThenRight),
					I18n::Get(I18n::Option::LaserMouseDirectionDownThenRight),
				}).setKeyTextureIdx(9),
				CreateInfo::Int(ConfigIni::Key::kLaserSignalSensitivity, kLaserSignalSensitivityMin, kLaserSignalSensitivityMax, kLaserSignalSensitivityDefault).setKeyTextureIdx(10), // TODO: additional suffix for zero value
				CreateInfo::Enum(ConfigIni::Key::kSwapLaserLR, Array<StringView>{
					I18n::Get(I18n::Option::Off),
					I18n::Get(I18n::Option::On),
				}).setKeyTextureIdx(11),
				CreateInfo::Enum(ConfigIni::Key::kSelectCloseFolderKey, Array<StringView>{
					I18n::Get(I18n::Option::SelectCloseFolderKeyBackspace),
					I18n::Get(I18n::Option::SelectCloseFolderKeyEsc),
				}).setKeyTextureIdx(12),
				CreateInfo::Enum(ConfigIni::Key::kUse3BTsPlusStartAsBack, Array<StringView>{
					I18n::Get(I18n::Option::Off),
					I18n::Get(I18n::Option::On),
				}).setKeyTextureIdx(13),
			}),
			OptionMenu(OptionTexture::kMenuKeyValueOther, {
				CreateInfo::Enum(ConfigIni::Key::kHispeedShowXMod, Array<StringView>{
					I18n::Get(I18n::Option::HispeedTypeHide),
					I18n::Get(I18n::Option::HispeedTypeShow),
				}),
				CreateInfo::Enum(ConfigIni::Key::kHispeedShowOMod, Array<StringView>{
					I18n::Get(I18n::Option::HispeedTypeHide),
					I18n::Get(I18n::Option::HispeedTypeShow),
				}),
				CreateInfo::Enum(ConfigIni::Key::kHispeedShowCMod, Array<StringView>{
					I18n::Get(I18n::Option::HispeedTypeHide),
					I18n::Get(I18n::Option::HispeedTypeShow),
				}),
				/*CreateInfo::Enum(ConfigIni::Key::kHideMouseCursor, Array<StringView>{
					I18n::Get(I18n::Option::HideMouseCursorOff),
					I18n::Get(I18n::Option::HideMouseCursorOn),
				}).setKeyTextureIdx(5),*/
				CreateInfo::Enum(ConfigIni::Key::kUseNumpadAsArrowKeys, Array<StringView>{
					I18n::Get(I18n::Option::UseNumpadAsArrowKeysOff),
					I18n::Get(I18n::Option::UseNumpadAsArrowKeysOnKeyboard),
					I18n::Get(I18n::Option::UseNumpadAsArrowKeysOnController),
				}).setKeyTextureIdx(6),
			}),
			OptionMenu(OptionTexture::kMenuKeyValueOther/*FIXME*/, {
			}),
		};
	}
}

OptionScene::OptionScene()
	: m_bgTexture(TextureAsset(OptionTexture::kBG))
	, m_headerTiledTexture(OptionTexture::kMenuHeader,
		{
			.row = kOptionMenuTypeEnumCount + 1,
			.sourceScale = SourceScale::k2x,
			.sourceSize = { 540, 48 },
		})
	, m_optionMenus(MakeOptionMenus())
{
	m_bgmStream.play();
	AutoMuteAddon::SetEnabled(true);
}

void OptionScene::update()
{
	if (m_currentOptionMenuIdx.has_value())
	{
		if (m_currentOptionMenuIdx == OptionMenuType::kKeyConfig)
		{
			m_keyConfigMenu.update();
		}
		else
		{
			m_optionMenus[*m_currentOptionMenuIdx].update();
		}

		if (KeyConfig::Down(kButtonBack))
		{
			// ボタン編集中の場合は戻らない
			if (m_currentOptionMenuIdx == OptionMenuType::kKeyConfig && m_keyConfigMenu.isButtonEditingState())
			{
				// 何もしない
			}
			else
			{
				m_currentOptionMenuIdx = none;
			}
		}
	}
	else
	{
		m_topMenu.update();

		if (KeyConfig::Down(kButtonStart))
		{
			m_currentOptionMenuIdx = m_topMenu.cursorAs<OptionMenuType>();
		}
		else if (KeyConfig::Down(kButtonBack))
		{
			exitScene();
		}
	}
}

void OptionScene::draw() const
{
	FitToHeight(m_bgTexture).drawAt(Scene::Center());

	StringView footerStr;
	if (m_currentOptionMenuIdx.has_value())
	{
		// Header
		const int32 headerIdx = *m_currentOptionMenuIdx + 1;
		m_headerTiledTexture(headerIdx).draw(Scaled(kHeaderX), Scaled(kHeaderY));

		if (m_currentOptionMenuIdx == OptionMenuType::kKeyConfig)
		{
			// Key config
			m_keyConfigMenu.draw();
			footerStr = I18n::Get(I18n::Option::GuideKeyConfig);
		}
		else
		{
			// Menu items
			m_optionMenus[*m_currentOptionMenuIdx].draw(LeftMarginVec() + Vec2{ ScaledByWidth(kOptionMenuOffsetX), Scaled(kOptionMenuOffsetY) });
			footerStr = I18n::Get(I18n::Option::GuideOption);
		}
	}
	else
	{
		// Top menu
		m_topMenu.draw();

		footerStr = I18n::Get(I18n::Option::GuideTop);
	}

	// Footer text
	m_font(footerStr).draw(Scaled(18), ScaledByWidth(kGuideX), Scaled(kGuideY), Palette::White);
}

Co::Task<void> OptionScene::fadeIn()
{
	co_await Co::ScreenFadeIn(kFadeDuration);
}

Co::Task<void> OptionScene::fadeOut()
{
	m_bgmStream.setFadeOut(kFadeDuration);
	co_await Co::ScreenFadeOut(kFadeDuration);
}

void OptionScene::exitScene()
{
	ConfigIni::Save();

	// Vsync設定を反映("0;120"または"1"の形式)
	const Array<String> vsyncParts = String{ ConfigIni::GetString(ConfigIni::Key::kVsync, U"0;300") }.split(U';');
	const bool vsyncEnabled = vsyncParts[0] == U"1";
	Graphics::SetVSyncEnabled(vsyncEnabled);

	// フレームレート制限(Vsync有効時は無効化)
	const int32 fpsLimitValue = vsyncParts.size() >= 2 ? ParseOr<int32>(vsyncParts[1], 300) : 300;
	const Optional<int32> frameRateLimit = vsyncEnabled ? none : Optional<int32>(fpsLimitValue);
	Addon::GetAddon<FrameRateLimit>(FrameRateLimit::kAddonName)->setTargetFPS(frameRateLimit);

	// 画面サイズ反映
	ApplyScreenSizeConfig();

	// シーン遷移
	requestNextScene<TitleScene>(TitleMenuItem::kOption);
}
