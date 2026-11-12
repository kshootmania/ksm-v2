#include "SelectFolderLoading.hpp"
#include "Common/FsUtils.hpp"
#include "Input/KeyConfig.hpp"

namespace
{
	// 1フレームあたりの読み込み時間
	constexpr Duration kLoadingTimeLimitPerFrame = 0.03s;
}

SelectFolderLoading::SelectFolderLoading(SelectMenuCacheDb::Loader&& loader)
	: Co::SequenceBase<bool>(Co::Layer::Modal)
	, m_loader(std::move(loader))
	, m_canvas(noco::Canvas::LoadFromFile(FsUtils::GetResourcePath(U"ui/parts/select_folder_loading.noco")))
{
}

Co::Task<bool> SelectFolderLoading::start()
{
	updateCanvas();

	while (!m_loader.isFinished())
	{
		co_await Co::NextFrame();

		// Escでキャンセル
		if (KeyConfig::Down(kButtonBack))
		{
			co_return false;
		}

		m_loader.advance(kLoadingTimeLimitPerFrame);
		updateCanvas();
	}

	co_return true;
}

void SelectFolderLoading::draw() const
{
	if (!m_canvas)
	{
		return;
	}

	m_canvas->draw();
}

void SelectFolderLoading::updateCanvas()
{
	if (!m_canvas)
	{
		return;
	}

	// 全譜面数が確定するまでは進捗を表示しない
	const auto progress = m_loader.progress();
	m_canvas->setParamValue(U"progressText", progress ? U"{}/{}"_fmt(progress->loadedChartCount, progress->totalChartCount) : String{});

	m_canvas->update(noco::HitTestEnabledYN::No);
}
