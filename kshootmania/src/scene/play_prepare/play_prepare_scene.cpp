#include "play_prepare_scene.hpp"
#include "scene/select/select_scene.hpp"
#include "scene/play/play_scene.hpp"

namespace
{
	constexpr Duration kFadeDuration = 1s;

	Co::Task<void> waitAsync()
	{
		co_await Co::Delay(2s);
	}
}

PlayPrepareScene::PlayPrepareScene(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay)
	: m_playSceneFactory(Co::MakeSceneFactory<PlayScene>(FilePath{ chartFilePath }, isAutoPlay))
{
}

Co::Task<Co::SceneFactory> PlayPrepareScene::start()
{
	m_seStream.play();

	// TODO: ジャケット画像の描画を追加してスケールをtweenerで操作
	const auto _ = Co::Linear(1s, [](double t) { Print << t; }).runScoped();

	const auto [isWait, isStart, isBack] = co_await Co::Any(
		waitAsync(),
		KeyConfig::WaitForDown(KeyConfig::kStart),
		KeyConfig::WaitForDown(KeyConfig::kBack));

	if (isBack)
	{
		co_return Co::MakeSceneFactory<SelectScene>();
	}
	else
	{
		co_return m_playSceneFactory;
	}
}

void PlayPrepareScene::draw() const
{
	FitToHeight(m_bgTexture).drawAt(Scene::Center());
}

Co::Task<void> PlayPrepareScene::fadeIn()
{
	co_await Co::SimpleFadeIn(kFadeDuration, Palette::White);
}
