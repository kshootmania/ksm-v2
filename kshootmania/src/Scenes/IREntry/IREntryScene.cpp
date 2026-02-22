#include "IREntryScene.hpp"
#include "Scenes/Select/SelectScene.hpp"
#include "Scenes/Common/ShowLoadingOneFrame.hpp"

namespace
{
	constexpr Duration kFadeDuration = 0.5s;

	FilePath GetIREntrySceneUIFilePath()
	{
		return FsUtils::GetResourcePath(U"ui/scene/ir_entry.noco");
	}

	std::shared_ptr<noco::Canvas> LoadIREntrySceneCanvas()
	{
		const FilePath uiFilePath = GetIREntrySceneUIFilePath();
		const auto canvas = noco::Canvas::LoadFromFile(uiFilePath);
		if (!canvas)
		{
			throw Error{ U"Failed to load '{}'"_fmt(uiFilePath) };
		}
		return canvas;
	}
}

IREntryScene::IREntryScene(std::shared_ptr<ksmaudio::Stream> bgmStream)
	: m_bgmStream(std::move(bgmStream))
	, m_canvas(LoadIREntrySceneCanvas())
	, m_menu(m_canvas)
{
}

Co::Task<void> IREntryScene::start()
{
	const auto updateRunner = Co::UpdaterTask([this] { update(); }).runScoped();

	// メニューが選択されるまで待機
	co_await m_menu.selectedMenuItemAsync();

	// 効果音を鳴らす
	m_bgmStream->setVolume(0.0);
	CommonSEAddon::Play(CommonSEType::kTitleEnter);
}

void IREntryScene::update()
{
	m_menu.update();
	m_canvas->update();
}

void IREntryScene::draw() const
{
	m_canvas->draw();
}

Co::Task<void> IREntryScene::fadeIn()
{
	const auto canvasUpdateRunner = Co::UpdaterTask([this] { m_canvas->update(); }).runScoped();

	co_await Co::ScreenFadeIn(kFadeDuration);
}

Co::Task<void> IREntryScene::fadeOut()
{
	const auto canvasUpdateRunner = Co::UpdaterTask([this] { m_canvas->update(); }).runScoped();

	// Note: ソースコード公開用にIR関連処理は削除済み
	co_await Co::ScreenFadeOut(kFadeDuration);
	requestNextScene<SelectScene>();

	// SelectSceneはコンストラクタの処理に時間がかかるので、ローディングはここで出しておく
	co_await ShowLoadingOneFrame::Play(HasBgYN::No);
}
