#include "title_scene.hpp"
#include "title_assets.hpp"

TitleScene::TitleScene(const InitData& initData)
	: MyScene(initData)
	, m_menu(this)
	, m_bgTexture(TextureAsset(TitleTexture::kBG))
{
	m_bgmStream.play();
	ScreenFadeAddon::FadeIn();
	AutoMuteAddon::SetEnabled(true);
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
	m_bgmStream.setVolume(0.0);
	m_enterSe.play(); // TODO: シーン遷移時に消えるのを修正

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
