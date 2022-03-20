#include "select_scene.hpp"
#include "select_assets.hpp"

SelectScene::SelectScene(const InitData& initData)
	: SceneManager<StringView>::Scene(initData)
	, m_bgTexture(SelectTexture::kBG)
{
}

void SelectScene::update()
{
}

void SelectScene::draw() const
{
}
