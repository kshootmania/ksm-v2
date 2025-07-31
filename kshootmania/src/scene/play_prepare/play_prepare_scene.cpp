#include "play_prepare_scene.hpp"
#include "scene/select/select_scene.hpp"
#include "scene/play/play_scene.hpp"
#include "scene/common/show_loading_one_frame.hpp"

namespace
{
	constexpr Duration kFadeDuration = 1s;

	constexpr SizeF kJacketSize{ 300.0, 300.0 };
}

PlayPrepareScene::PlayPrepareScene(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay)
	: m_chartFilePath(chartFilePath)
	, m_isAutoPlay(isAutoPlay)
	, m_chartData(kson::LoadKSHChartData(chartFilePath.narrow()))
	, m_jacketTexture(FileSystem::ParentPath(chartFilePath) + Unicode::FromUTF8(m_chartData.meta.jacketFilename))
{
}

Co::Task<void> PlayPrepareScene::start()
{
	m_seStream.play();

	// ジャケットのスケールアニメーション
	const auto _ = Co::Ease(&m_jacketScale, 2s).fromTo(1.2, 1.0).play().runScoped();

	const auto [isWait, isStart, isBack] = co_await Co::Any(
		Co::Delay(2s),
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
	auto center = Scene::Center();

	FitToHeight(m_bgTexture).drawAt(center);

	// 曲名を表示
	const Font titleFontBold = AssetManagement::SystemFontBold();
	titleFontBold(Unicode::FromUTF8(m_chartData.meta.title)).drawAt(27, Vec2(center.x, center.y + 85));

	// アーティストを表示
	const Font artistFont = AssetManagement::SystemFont();
	artistFont(Unicode::FromUTF8(m_chartData.meta.artist)).drawAt(20, Vec2(center.x, center.y + 140));

	// ジャケットを表示
	const SizeF jacketSize = kJacketSize * m_jacketScale;
	m_jacketTexture.resized(jacketSize).drawAt(center.movedBy(0, Scaled(-100)));

	// 難易度を表示
	const TiledTexture difficultyTiledTexture(PlayPrepareTexture::kDifficulty, TiledTextureSizeInfo{
		.row = kNumDifficulties,
		.sourceSize = { 126, 36 }
	});
	difficultyTiledTexture(m_chartData.meta.difficulty.idx).scaled(0.8).draw(Point{94, 400});

}

Co::Task<void> PlayPrepareScene::fadeIn()
{
	co_await Co::ScreenFadeIn(kFadeDuration, Palette::White);
}
