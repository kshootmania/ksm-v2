#include "result_scene.hpp"

ResultScene::ResultScene(const InitData& initData)
	: MyScene(initData)
	, m_chartData(getData().resultSceneArgs.chartData)
	, m_playResult(getData().resultSceneArgs.playResult)
{
}

void ResultScene::update()
{
	// StartボタンまたはBackボタンで楽曲選択画面に戻る
	if (KeyConfig::Down(KeyConfig::kStart) || KeyConfig::Down(KeyConfig::kBack))
	{
		changeScene(SceneName::kSelect, kDefaultTransitionMs);
	}
}

void ResultScene::updateFadeIn(double t)
{
	update();
}

void ResultScene::draw() const
{
	ScreenUtils::FitToHeight(m_bgTexture).drawAt(Scene::Center());
}

void ResultScene::drawFadeIn(double t) const
{
	draw();

	// 画面全体を白色でフェード
	Scene::Rect().draw(ColorF(1.0, 1.0 - t));
}
