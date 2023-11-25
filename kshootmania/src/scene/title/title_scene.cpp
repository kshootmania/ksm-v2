#include "title_scene.hpp"
#include "title_assets.hpp"

namespace
{
	constexpr double kExitFadeOutDurationSec = 0.8;
}

TitleScene::TitleScene(const InitData& initData)
	: MyScene(initData)
	, m_menu(this)
	, m_bgTexture(TextureAsset(TitleTexture::kBG))
	, m_exitStopwatch(StartImmediately::No)
{
}

void TitleScene::update()
{
	if (m_exitStopwatch.isStarted() && m_exitStopwatch.sF() > kExitFadeOutDurationSec)
	{
		System::Exit();
		return;
	}

	m_menu.update();
}

void TitleScene::draw() const
{
	FitToHeight(m_bgTexture).drawAt(Scene::Center());
	m_menu.draw();

	// 終了前のフェードアウト描画
	if (m_exitStopwatch.isStarted())
	{
		const double alpha = m_exitStopwatch.sF() / kExitFadeOutDurationSec;
		Scene::Rect().draw(ColorF{ 0.0, alpha });
	}
}

void TitleScene::processMenuItem(TitleMenu::Item item)
{
	switch (item)
	{
	case TitleMenu::kStart:
		// TODO: Internet Rankingへのサインイン
		changeScene(SceneName::kSelect, kDefaultTransitionMs);
		break;

	case TitleMenu::kOption:
		changeScene(SceneName::kOption, kDefaultTransitionMs);
		break;

	case TitleMenu::kInputGate:
		break;

	case TitleMenu::kExit:
		m_exitStopwatch.start();
		break;
	}
}
