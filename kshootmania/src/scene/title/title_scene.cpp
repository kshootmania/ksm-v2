#include "title_scene.hpp"
#include "scene/select/select_scene.hpp"
#include "scene/option/option_scene.hpp"
#include "scene/common/show_loading_one_frame.hpp"

namespace
{
	constexpr Duration kFadeDuration = 0.5s;
	constexpr Duration kFadeDurationExit = 0.8s;
}

TitleScene::TitleScene(TitleMenuItem defaultMenuitem)
	: m_menu(defaultMenuitem)
{
}

Co::Task<void> TitleScene::start()
{
	const Co::ScopedUpdater updater{ [this] { update(); } };

	m_bgmStream.play();
	AutoMuteAddon::SetEnabled(true);

	// メニューが選択されるまで待機
	m_selectedMenuItem = co_await m_menu.selectedMenuItemAsync();

	// 効果音を鳴らす
	m_bgmStream.setVolume(0.0);
	m_enterSe.play(); // TODO: シーン遷移時に消えるのを修正
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
	co_await Co::ScreenFadeIn(kFadeDuration);
}

Co::Task<void> TitleScene::fadeOut()
{
	// 次のシーンへ遷移
	switch (m_selectedMenuItem)
	{
	case TitleMenuItem::kStart:
		co_await Co::ScreenFadeOut(kFadeDuration);
		requestNextScene<SelectScene>();

		// SelectSceneはコンストラクタの処理に時間がかかるので、ローディングはここで出しておく
		co_await ShowLoadingOneFrame::Play(HasBgYN::No);
		break;

	case TitleMenuItem::kOption:
		co_await Co::ScreenFadeOut(kFadeDuration);
		requestNextScene<OptionScene>();
		break;

	case TitleMenuItem::kInputGate:
		// TODO: INPUT GATEへ遷移
		co_await Co::ScreenFadeOut(kFadeDuration);
		requestNextScene<TitleScene>(TitleMenuItem::kInputGate);
		break;

	case TitleMenuItem::kExit:
		// 効果音が途中で切れないよう終了時は少し長めにフェードアウト
		co_await Co::ScreenFadeOut(kFadeDurationExit);
		break;

	default:
		throw Error{ U"Invalid menu item: {}"_fmt(std::to_underlying(m_selectedMenuItem)) };
	}
}
