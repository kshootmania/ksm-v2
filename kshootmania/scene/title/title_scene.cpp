#include "title_scene.hpp"
#include <screen_utils.hpp>

const String TitleScene::kSceneName = U"Title";

TitleScene::TitleScene(const InitData& initData)
	: SceneManager<String>::Scene(initData)
	, m_bgTexture(U"imgs/standby.jpg")
	, m_menu(this)
{
}

void TitleScene::update()
{
	m_menu.update();
}

void TitleScene::draw() const
{
	ScreenUtils::FitToHeight(m_bgTexture).drawAt(Scene::Center());
}
