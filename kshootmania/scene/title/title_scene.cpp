#include "title_scene.hpp"
#include <screen_utils.hpp>

const String TitleScene::kSceneName = U"Title";

TitleScene::TitleScene(const InitData& initData)
	: SceneManager<String>::Scene(initData)
	, m_bgTexture(U"imgs/standby.jpg")
	, m_menu(MakeVerticalSimpleMenu(this, kMenuItemTypeMax, false))
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

void TitleScene::enterKeyPressed(const MenuEvent& event)
{
	Print << event.menuItemIdx;
}

void TitleScene::escKeyPressed(const MenuEvent&)
{
	m_menu.moveCursorTo(kExit);
}
