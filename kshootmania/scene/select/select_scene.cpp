#include "select_scene.hpp"
#include "select_assets.hpp"

SelectScene::SelectScene(const InitData& initData)
	: SceneManager<StringView>::Scene(initData)
	, m_bgTexture(TextureAsset(SelectTexture::kBG))
{
}

void SelectScene::update()
{
}

void SelectScene::draw() const
{
	ScreenUtils::FitToHeight(m_bgTexture).drawAt(Scene::Center());
	m_bgAnim.draw();
}
