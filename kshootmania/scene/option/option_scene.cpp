#include "option_scene.hpp"
#include "option_assets.hpp"
#include "i18n/i18n.hpp"
#include "ini/config_ini.hpp"

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

	std::array<OptionMenu, OptionScene::kOptionMenuTypeEnumCount> MakeOptionMenus()
	{
		using StrPair = std::pair<String, String>;
		using IntStrPair = std::pair<int32, String>;
		using DoubleStrPair = std::pair<double, String>;

		Array<StrPair> availableLanguageStrPairs;
		for (const auto& language : I18n::GetAvailableLanguageList())
		{
			availableLanguageStrPairs.emplace_back(language, language);
		}

		// FIXME
		return {
			OptionMenu(OptionTexture::kMenuKeyValueDisplaySound, {
				OptionMenuFieldCreateInfo(ConfigIni::Key::kFullScreen, Array<StringView>{
					I18n::Get(I18n::Option::kFullscreenOff),
					I18n::Get(I18n::Option::kFullscreenOn),
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kWindowResolution, Array<StrPair>{
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
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kFullScreenResolution, Array<StrPair>{
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
				OptionMenuFieldCreateInfo(ConfigIni::Key::kLanguage, availableLanguageStrPairs),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kTextureSize, Array<IntStrPair>{
					IntStrPair{ ConfigIni::Value::TextureSize::kSmall, I18n::Get(I18n::Option::kTextureSizeSmall) },
					IntStrPair{ ConfigIni::Value::TextureSize::kMedium, I18n::Get(I18n::Option::kTextureSizeMedium) },
					IntStrPair{ ConfigIni::Value::TextureSize::kLarge, I18n::Get(I18n::Option::kTextureSizeLarge) },
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kBGDisplayMode, Array<IntStrPair>{
					IntStrPair{ ConfigIni::Value::BGDisplayMode::kHide, I18n::Get(I18n::Option::kBGDisplayModeHide) },
					IntStrPair{ ConfigIni::Value::BGDisplayMode::kShowNoAnim, I18n::Get(I18n::Option::kBGDisplayModeShowNoAnim) },
					IntStrPair{ ConfigIni::Value::BGDisplayMode::kShowAnim, I18n::Get(I18n::Option::kBGDisplayModeShowAnim) },
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kAlwaysShowOtherFolders, Array<StringView>{
					I18n::Get(I18n::Option::kAlwaysShowOtherFoldersOff),
					I18n::Get(I18n::Option::kAlwaysShowOtherFoldersOn),
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kHideAllFolder, Array<StringView>{
					I18n::Get(I18n::Option::kHideAllFolderOff),
					I18n::Get(I18n::Option::kHideAllFolderOn),
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kMasterVolume, Array<IntStrPair>{
					IntStrPair{ 100, U"100" + I18n::Get(I18n::Option::kMasterVolumePercent) },
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kVsync, Array<StringView>{
					I18n::Get(I18n::Option::kVsyncOff),
					I18n::Get(I18n::Option::kVsyncOn),
				}),
			}),
			OptionMenu(OptionTexture::kMenuKeyValueInputJudgment, {
			}),
			OptionMenu(OptionTexture::kMenuKeyValueOther, {
			}),
			OptionMenu(OptionTexture::kMenuKeyValueOther/*FIXME*/, {
			}),
		};
	}
}

OptionScene::OptionScene(const InitData& initData)
	: SceneManager<StringView>::Scene(initData)
	, m_bgTexture(TextureAsset(OptionTexture::kBG))
	, m_headerTextureAtlas(OptionTexture::kMenuHeader, kOptionMenuTypeEnumCount + 1)
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

	StringView guideStr;
	if (m_currentOptionMenuIdx.has_value())
	{
		// Header
		const int32 headerIdx = *m_currentOptionMenuIdx + 1;
		m_headerTextureAtlas(headerIdx).resized(Scaled(kHeaderWidth), Scaled(kHeaderHeight)).draw(Scaled(kHeaderX), Scaled(kHeaderY));

		// Menu items
		m_optionMenus[*m_currentOptionMenuIdx].draw(LeftMarginVec() + Vec2{ ScaledByWidth(kOptionMenuOffsetX), Scaled(kOptionMenuOffsetY) });

		guideStr = I18n::Get(I18n::Option::kGuideOption);
	}
	else
	{
		// Top menu
		m_topMenu.draw();

		guideStr = I18n::Get(I18n::Option::kGuideTop);
	}

	// Footer guide
	m_font(guideStr).draw(ScaledByWidth(kGuideX), Scaled(kGuideY), Palette::White);
}

void OptionScene::exitScene()
{
	changeScene(TitleScene::kSceneName, SceneManagement::kDefaultTransitionMs);
}
