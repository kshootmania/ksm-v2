#include "title_scene.hpp"
#include "title_assets.hpp"

TitleScene::TitleScene(const InitData& initData)
	: MyScene(initData)
	, m_menu(this)
	, m_bgTexture(TextureAsset(TitleTexture::kBG))
{
	ScreenFadeAddon::FadeIn();
}

void TitleScene::update()
{
	if (ScreenFadeAddon::IsFadingOut())
	{
		return;
	}

	m_menu.update();
}

void TitleScene::draw() const
{
	FitToHeight(m_bgTexture).drawAt(Scene::Center());
	m_menu.draw();
}

void TitleScene::processMenuItem(TitleMenu::Item item)
{
	switch (item)
	{
	case TitleMenu::kStart:
		// TODO: Internet Rankingへのサインイン
		ScreenFadeAddon::FadeOutToScene(SceneName::kSelect);
		break;

	case TitleMenu::kOption:
		ScreenFadeAddon::FadeOutToScene(SceneName::kOption);
		break;

	case TitleMenu::kInputGate:
		break;

	case TitleMenu::kExit:
		ScreenFadeAddon::FadeOut(
			[]
			{
				System::Exit();
			});
		break;
	}
}
