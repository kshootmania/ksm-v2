#include "option_scene.hpp"
#include "option_assets.hpp"
#include "i18n.hpp"

namespace
{
	int32 kGuideX = 20;
	int32 kGuideY = 456;
}

OptionScene::OptionScene(const InitData& initData)
	: SceneManager<StringView>::Scene(initData)
	, m_bgTexture(TextureAsset(OptionTexture::kBG))
	, m_topMenu(this)
	, m_font(ScreenUtils::Scaled(18), FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc")
{
}

void OptionScene::update()
{
	m_topMenu.update();
}

void OptionScene::draw() const
{
	ScreenUtils::FitToHeight(m_bgTexture).drawAt(Scene::Center());

	if (m_topMenu.active())
	{
		m_topMenu.draw();

		m_font(I18n::Get(I18n::Option::kGuideTop)).draw(ScreenUtils::ScaledByWidth(kGuideX), ScreenUtils::Scaled(kGuideY), Palette::White);
	}
	else
	{
		m_font(I18n::Get(I18n::Option::kGuideOption)).draw(ScreenUtils::ScaledByWidth(kGuideX), ScreenUtils::Scaled(kGuideY), Palette::White);
	}
}

void OptionScene::exitScene()
{
	changeScene(TitleScene::kSceneName, SceneManagement::kDefaultTransitionMs);
}
