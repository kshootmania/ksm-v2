#include "title_scene.hpp"
#include "title_assets.hpp"

TitleScene::TitleScene(const InitData& initData)
	: SceneManager<StringView>::Scene(initData)
	, m_bgTexture(TextureAsset(TitleTexture::kBG))
{
}

void TitleScene::update()
{
	m_menu.update();
}

void TitleScene::draw() const
{
	ScreenUtils::FitToHeight(m_bgTexture).drawAt(Scene::Center());
	m_menu.draw();
}
