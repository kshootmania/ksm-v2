#include "TitleScene.hpp"

#define NO_IR_ENTRY_SCENE // Note: ソースコード公開用にIR関連処理は削除済み

#ifndef NO_IR_ENTRY_SCENE
#include "Scenes/IREntry/IREntryScene.hpp"
#endif
#include "Scenes/Select/SelectScene.hpp"
#include "Scenes/Option/OptionScene.hpp"
#include "Scenes/Common/ShowLoadingOneFrame.hpp"

namespace
{
	constexpr Duration kFadeDuration = 0.5s;
	constexpr Duration kFadeDurationExit = 0.8s;

	FilePath GetTitleSceneUIFilePath()
	{
		return FsUtils::GetResourcePath(U"ui/scene/title.noco");
	}

	std::shared_ptr<noco::Canvas> LoadTitleSceneCanvas()
	{
		const FilePath uiFilePath = GetTitleSceneUIFilePath();
		const auto canvas = noco::Canvas::LoadFromFile(uiFilePath);
		if (!canvas)
		{
			throw Error{ U"Failed to load '{}'"_fmt(uiFilePath) };
		}
		return canvas;
	}
}

TitleScene::TitleScene(TitleMenuItem defaultMenuitem)
	: m_bgmStream(std::make_shared<ksmaudio::Stream>("se/title_bgm.ogg", 1.0, false, false, true))
	, m_canvas(LoadTitleSceneCanvas())
	, m_menu(defaultMenuitem, m_canvas)
{
}

TitleScene::TitleScene(TitleMenuItem defaultMenuitem, std::shared_ptr<ksmaudio::Stream> bgmStream)
	: m_bgmStream(std::move(bgmStream))
	, m_canvas(LoadTitleSceneCanvas())
	, m_menu(defaultMenuitem, m_canvas)
{
}

Co::Task<void> TitleScene::start()
{
	const auto updateRunner = Co::UpdaterTask([this] { update(); }).runScoped();

	if (!m_bgmStream->isPlaying())
	{
		m_bgmStream->play();
	}
	AutoMuteAddon::SetEnabled(true);

	// メニューが選択されるまで待機
	m_selectedMenuItem = co_await m_menu.selectedMenuItemAsync();

	// 効果音を鳴らす
#ifdef NO_IR_ENTRY_SCENE
	m_bgmStream->setVolume(0.0);
#else
	if (m_selectedMenuItem != TitleMenuItem::kStart)
	{
		m_bgmStream->setVolume(0.0);
	}
#endif
	CommonSEAddon::Play(CommonSEType::kTitleEnter);
}

void TitleScene::update()
{
	m_menu.update();
	m_canvas->update();
}

void TitleScene::draw() const
{
	m_canvas->draw();
}

Co::Task<void> TitleScene::fadeIn()
{
	const auto canvasUpdateRunner = Co::UpdaterTask([this] { m_canvas->update(); }).runScoped();

	co_await Co::ScreenFadeIn(kFadeDuration);
}

Co::Task<void> TitleScene::fadeOut()
{
	const auto canvasUpdateRunner = Co::UpdaterTask([this] { m_canvas->update(); }).runScoped();

	// 次のシーンへ遷移
	switch (m_selectedMenuItem)
	{
	case TitleMenuItem::kStart:
		co_await Co::ScreenFadeOut(kFadeDuration);
#ifdef NO_IR_ENTRY_SCENE
		requestNextScene<SelectScene>();
		// SelectSceneはコンストラクタの処理に時間がかかるので、ローディングはここで出しておく
		co_await ShowLoadingOneFrame::Play(HasBgYN::No);
#else
		requestNextScene<IREntryScene>(m_bgmStream);
#endif
		break;

	case TitleMenuItem::kOption:
		co_await Co::ScreenFadeOut(kFadeDuration);
		requestNextScene<OptionScene>();
		break;

	case TitleMenuItem::kInputGate:
		// Note: ソースコード公開用にINPUT GATE関連処理は削除済み
		co_await Co::ScreenFadeOut(kFadeDuration);
		MessageBoxUtils::ShowOK(U"This version does not support INPUT GATE.");
		requestNextScene<TitleScene>(TitleMenuItem::kInputGate);
		break;

	case TitleMenuItem::kExit:
		// 効果音が途中で切れないよう終了時は少し長めにフェードアウト
		co_await Co::ScreenFadeOut(kFadeDurationExit);
		break;

	default:
		throw Error{ U"Invalid menu item: {}"_fmt(static_cast<std::underlying_type_t<TitleMenuItem>>(m_selectedMenuItem)) };
	}
}
