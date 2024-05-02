#include "title_scene.hpp"
#include "scene/select/select_scene.hpp"
#include "scene/option/option_scene.hpp"

namespace
{
	constexpr Duration kFadeDuration = 0.5s;
	constexpr Duration kFadeDurationExit = 0.8s;
}

TitleScene::TitleScene(TitleMenuItem defaultMenuitem)
	: m_menu(defaultMenuitem)
{
}

Co::Task<Co::SceneFactory> TitleScene::start()
{
	const Co::ScopedUpdater updater{ [this] { update(); } };

	m_bgmStream.play();
	AutoMuteAddon::SetEnabled(true);

	// メニューが選択されるまで待機
	const TitleMenuItem selectedMenuItem = co_await m_menu.onSelect().firstAsync();

	// 効果音を鳴らす
	m_bgmStream.setVolume(0.0);
	m_enterSe.play(); // TODO: シーン遷移時に消えるのを修正

	// 次のシーンへ遷移
	switch (selectedMenuItem)
	{
	case TitleMenuItem::kStart:
		co_await Co::SimpleFadeOut(kFadeDuration);
		co_return Co::MakeSceneFactory<SelectScene>();

	case TitleMenuItem::kOption:
		co_await Co::SimpleFadeOut(kFadeDuration);
		co_return Co::MakeSceneFactory<OptionScene>();

	case TitleMenuItem::kInputGate:
		// TODO: INPUT GATEへ遷移
		co_await Co::SimpleFadeOut(kFadeDuration);
		co_return Co::MakeSceneFactory<TitleScene>(TitleMenuItem::kInputGate);

	case TitleMenuItem::kExit:
		// 効果音が途中で切れないよう終了時は少し長めにフェードアウト
		co_await Co::SimpleFadeOut(kFadeDurationExit);
		co_return Co::SceneFinish();
		
	default:
		throw Error{ U"Invalid menu item: {}"_fmt(std::to_underlying(selectedMenuItem)) };
	}

	co_return *m_nextSceneFactory;
}

void TitleScene::update()
{
	m_menu.update();
}

void TitleScene::draw() const
{
	FitToHeight(m_bgTexture).drawAt(Scene::Center());
	m_menu.draw();
}

Co::Task<void> TitleScene::fadeIn()
{
	co_await Co::SimpleFadeIn(kFadeDuration);
}
