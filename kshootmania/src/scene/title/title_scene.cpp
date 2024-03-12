#include "title_scene.hpp"

CoTask<TitleMenuItem> TitleScene::start()
{
	m_bgmStream.play();
	AutoMuteAddon::SetEnabled(true);

	const auto [selectedMenuItem, _] = co_await Co::WhenAll(m_menu.waitForSelection(), Co::FadeIn(0.6s));

	m_bgmStream.setVolume(0.0);
	m_enterSe.play(); // TODO: シーン遷移時に消えるのを修正

	co_await Co::FadeOut(0.6s);

	co_return selectedMenuItem;
}

void TitleScene::draw() const
{
	FitToHeight(m_bgTexture).drawAt(Scene::Center());
	m_menu.draw();
}
