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

		// FIXME
		return {
			OptionMenu(OptionTexture::kMenuKeyValueDisplaySound, {
				OptionMenuFieldCreateInfo(ConfigIni::Key::kFullScreen, Array<StrPair>{
					StrPair{ U"0", I18n::Get(I18n::Option::kFullscreenOff) },
					StrPair{ U"1", I18n::Get(I18n::Option::kFullscreenOn) },
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kFullScreen, Array<StrPair>{
					StrPair{ U"0", I18n::Get(I18n::Option::kFullscreenOff) },
					StrPair{ U"1", I18n::Get(I18n::Option::kFullscreenOn) },
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kFullScreen, Array<StrPair>{
					StrPair{ U"0", I18n::Get(I18n::Option::kFullscreenOff) },
					StrPair{ U"1", I18n::Get(I18n::Option::kFullscreenOn) },
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kFullScreen, Array<StrPair>{
					StrPair{ U"0", I18n::Get(I18n::Option::kFullscreenOff) },
					StrPair{ U"1", I18n::Get(I18n::Option::kFullscreenOn) },
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kFullScreen, Array<StrPair>{
					StrPair{ U"0", I18n::Get(I18n::Option::kFullscreenOff) },
					StrPair{ U"1", I18n::Get(I18n::Option::kFullscreenOn) },
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kFullScreen, Array<StrPair>{
					StrPair{ U"0", I18n::Get(I18n::Option::kFullscreenOff) },
					StrPair{ U"1", I18n::Get(I18n::Option::kFullscreenOn) },
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kFullScreen, Array<StrPair>{
					StrPair{ U"0", I18n::Get(I18n::Option::kFullscreenOff) },
					StrPair{ U"1", I18n::Get(I18n::Option::kFullscreenOn) },
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kFullScreen, Array<StrPair>{
					StrPair{ U"0", I18n::Get(I18n::Option::kFullscreenOff) },
					StrPair{ U"1", I18n::Get(I18n::Option::kFullscreenOn) },
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kFullScreen, Array<StrPair>{
					StrPair{ U"0", I18n::Get(I18n::Option::kFullscreenOff) },
					StrPair{ U"1", I18n::Get(I18n::Option::kFullscreenOn) },
				}),
				OptionMenuFieldCreateInfo(ConfigIni::Key::kFullScreen, Array<StrPair>{
					StrPair{ U"0", I18n::Get(I18n::Option::kFullscreenOff) },
					StrPair{ U"1", I18n::Get(I18n::Option::kFullscreenOn) },
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
	, m_font(ScreenUtils::Scaled(18), FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc")
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

	if (m_currentOptionMenuIdx.has_value())
	{
		// Header
		const int32 headerIdx = *m_currentOptionMenuIdx + 1;
		m_headerTextureAtlas(headerIdx).resized(Scaled(kHeaderWidth), Scaled(kHeaderHeight)).draw(Scaled(kHeaderX), Scaled(kHeaderY));

		// Menu items
		m_optionMenus[*m_currentOptionMenuIdx].draw(LeftMarginVec() + Vec2{ ScaledByWidth(kOptionMenuOffsetX), Scaled(kOptionMenuOffsetY) });

		// Guide
		m_font(I18n::Get(I18n::Option::kGuideOption)).draw(ScaledByWidth(kGuideX), Scaled(kGuideY), Palette::White);
	}
	else
	{
		// Top menu
		m_topMenu.draw();
		m_font(I18n::Get(I18n::Option::kGuideTop)).draw(ScaledByWidth(kGuideX), Scaled(kGuideY), Palette::White);
	}
}

void OptionScene::exitScene()
{
	changeScene(TitleScene::kSceneName, SceneManagement::kDefaultTransitionMs);
}
