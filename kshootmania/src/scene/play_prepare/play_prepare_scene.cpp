#include "play_prepare_scene.hpp"
#include "scene/select/select_scene.hpp"
#include "scene/play/play_scene.hpp"
#include "scene/common/show_loading_one_frame.hpp"

namespace
{
	constexpr Duration kFadeDuration = 1s;

	constexpr SizeF kJacketSize{ 300.0, 300.0 };

	constexpr Duration kJacketScaleDuration = 4s;
}

PlayPrepareScene::PlayPrepareScene(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay)
	: m_chartFilePath(chartFilePath)
	, m_isAutoPlay(isAutoPlay)
	, m_chartData(kson::LoadKSHChartData(chartFilePath.narrow()))
	, m_jacketTexture(FileSystem::ParentPath(chartFilePath) + Unicode::FromUTF8(m_chartData.meta.jacketFilename))
	, m_playPreparePanel(chartFilePath, m_chartData)
{
}

Co::Task<void> PlayPrepareScene::start()
{
	m_seStream.play();

	// ジャケットのスケールアニメーション
	const auto _ = Co::Ease(&m_jacketScale, kJacketScaleDuration).fromTo(1.2, 1.0).play().runScoped();

	const auto [isWait, isStart, isBack] = co_await Co::Any(
		Co::Delay(kJacketScaleDuration),
		KeyConfig::WaitForDown(KeyConfig::kStart),
		KeyConfig::WaitForDown(KeyConfig::kBack));

	if (isBack)
	{
		requestNextScene<SelectScene>();
	}
	else
	{
		co_await ShowLoadingOneFrame::Play(HasBgYN::No);
		requestNextScene<PlayScene>(m_chartFilePath, m_isAutoPlay);
	}
}

void PlayPrepareScene::draw() const
{
	FitToHeight(m_bgTexture).drawAt(Scene::Center());

	// ジャケットを表示
	const SizeF jacketSize = kJacketSize * m_jacketScale;
	m_jacketTexture.resized(jacketSize).drawAt(Scene::Center().movedBy(0, Scaled(-100)));

	// Infoパネルの表示
	m_playPreparePanel.draw();
}

Co::Task<void> PlayPrepareScene::fadeIn()
{
	co_await Co::ScreenFadeIn(kFadeDuration, Palette::White);
}
