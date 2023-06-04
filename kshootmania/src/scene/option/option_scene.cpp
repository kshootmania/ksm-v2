#include "option_scene.hpp"
#include "option_assets.hpp"

namespace
{
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
					I18n::Get(I18n::Option::kFullscreenOff),
					I18n::Get(I18n::Option::kFullscreenOn),
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
				CreateInfo::Enum(ConfigIni::Key::kLanguage, availableLanguageStrPairs),
				CreateInfo::Enum(ConfigIni::Key::kTextureSize, Array<IntStrPair>{
					IntStrPair{ ConfigIni::Value::TextureSize::kSmall, I18n::Get(I18n::Option::kTextureSizeSmall) },
					IntStrPair{ ConfigIni::Value::TextureSize::kMedium, I18n::Get(I18n::Option::kTextureSizeMedium) },
					IntStrPair{ ConfigIni::Value::TextureSize::kLarge, I18n::Get(I18n::Option::kTextureSizeLarge) },
				}),
				CreateInfo::Enum(ConfigIni::Key::kBGDisplayMode, Array<IntStrPair>{
					IntStrPair{ ConfigIni::Value::BGDisplayMode::kHide, I18n::Get(I18n::Option::kBGDisplayModeHide) },
					IntStrPair{ ConfigIni::Value::BGDisplayMode::kShowNoAnim, I18n::Get(I18n::Option::kBGDisplayModeShowNoAnim) },
					IntStrPair{ ConfigIni::Value::BGDisplayMode::kShowAnim, I18n::Get(I18n::Option::kBGDisplayModeShowAnim) },
				}),
				CreateInfo::Enum(ConfigIni::Key::kAlwaysShowOtherFolders, Array<StringView>{
					I18n::Get(I18n::Option::kAlwaysShowOtherFoldersOff),
					I18n::Get(I18n::Option::kAlwaysShowOtherFoldersOn),
				}),
				CreateInfo::Enum(ConfigIni::Key::kHideAllFolder, Array<StringView>{
					I18n::Get(I18n::Option::kHideAllFolderOff),
					I18n::Get(I18n::Option::kHideAllFolderOn),
				}),
				CreateInfo::Int(ConfigIni::Key::kMasterVolume, kMasterVolumeMin, kMasterVolumeMax, kMasterVolumeDefault, I18n::Get(I18n::Option::kMasterVolumePercent), 5),
				CreateInfo::Enum(ConfigIni::Key::kVsync, Array<StringView>{
					I18n::Get(I18n::Option::kVsyncOff),
					I18n::Get(I18n::Option::kVsyncOn),
				}),
			}),
			OptionMenu(OptionTexture::kMenuKeyValueInputJudgment, {
				CreateInfo::Enum(ConfigIni::Key::kJudgmentModeBT, Array<StringView>{
					I18n::Get(I18n::Option::kJudgmentOn),
					I18n::Get(I18n::Option::kJudgmentOff),
					I18n::Get(I18n::Option::kJudgmentAuto),
					I18n::Get(I18n::Option::kJudgmentHide),
				}),
				CreateInfo::Enum(ConfigIni::Key::kJudgmentModeFX, Array<StringView>{
					I18n::Get(I18n::Option::kJudgmentOn),
					I18n::Get(I18n::Option::kJudgmentOff),
					I18n::Get(I18n::Option::kJudgmentAuto),
					I18n::Get(I18n::Option::kJudgmentHide),
				}),
				CreateInfo::Enum(ConfigIni::Key::kJudgmentModeLaser, Array<StringView>{
					I18n::Get(I18n::Option::kJudgmentOn),
					I18n::Get(I18n::Option::kJudgmentOff),
					I18n::Get(I18n::Option::kJudgmentAuto),
					I18n::Get(I18n::Option::kJudgmentHide),
				}),
				CreateInfo::Enum(ConfigIni::Key::kLaserInputType, Array<IntStrPair>{
					IntStrPair{ ConfigIni::Value::LaserInputType::kKeyboard, I18n::Get(I18n::Option::kLaserInputTypeKeyboard) },
					IntStrPair{ ConfigIni::Value::LaserInputType::kSlider, I18n::Get(I18n::Option::kLaserInputTypeSlider) },
					IntStrPair{ ConfigIni::Value::LaserInputType::kMouseXY, I18n::Get(I18n::Option::kLaserInputTypeMouseXY) },
					IntStrPair{ ConfigIni::Value::LaserInputType::kAnalogStickXY, I18n::Get(I18n::Option::kLaserInputTypeAnalogStickXY) },
				}),
				CreateInfo::Enum(ConfigIni::Key::kAssistTick, Array<StringView>{
					I18n::Get(I18n::Option::kAssistTickOff),
					I18n::Get(I18n::Option::kAssistTickOn),
				}),
				CreateInfo::Enum(ConfigIni::Key::kDisableIME, Array<StringView>{
					I18n::Get(I18n::Option::kDisableIMEOff),
					I18n::Get(I18n::Option::kDisableIMEOnLow),
					I18n::Get(I18n::Option::kDisableIMEOnMid),
					I18n::Get(I18n::Option::kDisableIMEOnHigh),
				}),
				CreateInfo::Int(ConfigIni::Key::kTimingAdjust, kTimingAdjustMin, kTimingAdjustMax, kTimingAdjustDefault, I18n::Get(I18n::Option::kTimingAdjustMs)), // TODO: additional suffix
				CreateInfo::Int(ConfigIni::Key::kLaserTimingAdjust, kTimingAdjustMin, kTimingAdjustMax, kTimingAdjustDefault, I18n::Get(I18n::Option::kTimingAdjustMs)),
				CreateInfo::Enum(ConfigIni::Key::kLaserMouseDirectionX, Array<StringView>{
					I18n::Get(I18n::Option::kLaserMouseDirectionLeftThenRight),
					I18n::Get(I18n::Option::kLaserMouseDirectionRightThenRight),
				}),
				CreateInfo::Enum(ConfigIni::Key::kLaserMouseDirectionY, Array<StringView>{
					I18n::Get(I18n::Option::kLaserMouseDirectionUpThenRight),
					I18n::Get(I18n::Option::kLaserMouseDirectionDownThenRight),
				}),
				CreateInfo::Int(ConfigIni::Key::kLaserSignalSensitivity, kLaserSignalSensitivityMin, kLaserSignalSensitivityMax, kLaserSignalSensitivityDefault), // TODO: additional suffix for zero value
				CreateInfo::Enum(ConfigIni::Key::kSwapLaserLR, Array<StringView>{
					I18n::Get(I18n::Option::kDisabled),
					I18n::Get(I18n::Option::kEnabled),
				}),
				CreateInfo::Enum(ConfigIni::Key::kSelectCloseFolderKey, Array<StringView>{
					I18n::Get(I18n::Option::kSelectCloseFolderKeyBackspace),
					I18n::Get(I18n::Option::kSelectCloseFolderKeyEsc),
				}),
				CreateInfo::Enum(ConfigIni::Key::kUse3BTsPlusStartAsBack, Array<StringView>{
					I18n::Get(I18n::Option::kDisabled),
					I18n::Get(I18n::Option::kEnabled),
				}),
			}),
			OptionMenu(OptionTexture::kMenuKeyValueOther, {
				CreateInfo::Enum(ConfigIni::Key::kHispeedShowXMod, Array<StringView>{
					I18n::Get(I18n::Option::kHispeedTypeHide),
					I18n::Get(I18n::Option::kHispeedTypeShow),
				}),
				CreateInfo::Enum(ConfigIni::Key::kHispeedShowOMod, Array<StringView>{
					I18n::Get(I18n::Option::kHispeedTypeHide),
					I18n::Get(I18n::Option::kHispeedTypeShow),
				}),
				CreateInfo::Enum(ConfigIni::Key::kHispeedShowCMod, Array<StringView>{
					I18n::Get(I18n::Option::kHispeedTypeHide),
					I18n::Get(I18n::Option::kHispeedTypeShow),
				}),
				CreateInfo::Enum(ConfigIni::Key::kHideMouseCursor, Array<StringView>{
					I18n::Get(I18n::Option::kHideMouseCursorOff),
					I18n::Get(I18n::Option::kHideMouseCursorOn),
				}).setKeyTextureIdx(5),
				CreateInfo::Enum(ConfigIni::Key::kUseNumpadAsArrowKeys, Array<StringView>{
					I18n::Get(I18n::Option::kUseNumpadAsArrowKeysOff),
					I18n::Get(I18n::Option::kUseNumpadAsArrowKeysOnKeyboard),
					I18n::Get(I18n::Option::kUseNumpadAsArrowKeysOnController),
				}).setKeyTextureIdx(6),
			}),
			OptionMenu(OptionTexture::kMenuKeyValueOther/*FIXME*/, {
			}),
		};
	}
}

OptionScene::OptionScene(const InitData& initData)
	: MyScene(initData)
	, m_bgTexture(TextureAsset(OptionTexture::kBG))
	, m_headerTiledTexture(OptionTexture::kMenuHeader,
		{
			.row = kOptionMenuTypeEnumCount + 1,
			.sourceScale = ScreenUtils::SourceScale::k2x,
			.sourceSize = { 540, 48 },
		})
	, m_optionMenus(MakeOptionMenus())
	, m_font(ScreenUtils::Scaled(18), FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 0, FontStyle::Default)
{
}

void OptionScene::update()
{
	if (m_currentOptionMenuIdx.has_value())
	{
		m_optionMenus[*m_currentOptionMenuIdx].update();

		if (KeyConfig::Down(KeyConfig::kBack))
		{
			m_currentOptionMenuIdx = none;
		}
	}
	else
	{
		m_topMenu.update();

		if (KeyConfig::Down(KeyConfig::kStart))
		{
			m_currentOptionMenuIdx = m_topMenu.cursor<OptionMenuType>();
		}
		else if (KeyConfig::Down(KeyConfig::kBack))
		{
			exitScene();
		}
	}
}

void OptionScene::draw() const
{
	using namespace ScreenUtils;

	FitToHeight(m_bgTexture).drawAt(Scene::Center());

	StringView footerStr;
	if (m_currentOptionMenuIdx.has_value())
	{
		// Header
		const int32 headerIdx = *m_currentOptionMenuIdx + 1;
		m_headerTiledTexture(headerIdx).draw(Scaled(kHeaderX), Scaled(kHeaderY));

		// Menu items
		m_optionMenus[*m_currentOptionMenuIdx].draw(LeftMarginVec() + Vec2{ ScaledByWidth(kOptionMenuOffsetX), Scaled(kOptionMenuOffsetY) });

		footerStr = I18n::Get(I18n::Option::kGuideOption);
	}
	else
	{
		// Top menu
		m_topMenu.draw();

		footerStr = I18n::Get(I18n::Option::kGuideTop);
	}

	// Footer text
	m_font(footerStr).draw(ScaledByWidth(kGuideX), Scaled(kGuideY), Palette::White);
}

void OptionScene::exitScene()
{
	ConfigIni::Save();

	// 画面サイズ反映
	ScreenUtils::ApplyScreenSizeConfig();

	changeScene(SceneName::kTitle, kDefaultTransitionMs);
}
