#include "SearchInputDialog.hpp"
#include "Common/FsUtils.hpp"
#include "Input/KeyConfig.hpp"
#include "NocoExtensions/NocoUtils.hpp"
#include "Scenes/Common/ShowLoadingOneFrame.hpp"

namespace
{
	// 最終入力からインクリメンタルサーチが実行されるまでの遅延
	constexpr Duration kSearchTriggerDelay = 0.5s;
}

SearchInputDialog::SearchInputDialog(StringView initialQuery, std::function<void(StringView)> fnIncrementalUpdate)
	: m_initialQuery(initialQuery)
	, m_fnIncrementalUpdate(std::move(fnIncrementalUpdate))
{
}

Co::Task<Optional<String>> SearchInputDialog::start()
{
	const FilePath uiFilePath = FsUtils::GetResourcePath(U"ui/dialog/search_input_dialog.noco");
	m_canvas = noco::Canvas::LoadFromFile(uiFilePath);
	if (!m_canvas)
	{
		co_return none;
	}

	const auto searchBoxNode = NocoUtils::GetNodeByPath(m_canvas, { U"SearchDialog", U"SearchBox" });
	if (!searchBoxNode)
	{
		co_return none;
	}
	m_canvas->setTextValueByTag(U"searchQueryText", m_initialQuery);
	noco::CurrentFrame::SetFocusedNode(searchBoxNode);

	String prevFrameText = m_initialQuery;
	String lastAppliedText = m_initialQuery;
	Stopwatch triggerDelayStopwatch;
	bool isPending = false;

	const auto flushPendingUpdate = [&](StringView text) -> Co::Task<void>
	{
		if (text == lastAppliedText)
		{
			isPending = false;
			triggerDelayStopwatch.reset();
			co_return;
		}

		co_await ShowLoadingOneFrame::Play(LoadingBgMode::kNone);

		lastAppliedText = String{ text };
		if (m_fnIncrementalUpdate)
		{
			m_fnIncrementalUpdate(text);
		}
		isPending = false;
		triggerDelayStopwatch.reset();
	};

	while (true)
	{
		m_canvas->update();

		const String currentText = m_canvas->getTextValueByTag(U"searchQueryText");

		if (currentText != prevFrameText)
		{
			prevFrameText = currentText;
			isPending = true;
			triggerDelayStopwatch.restart();
		}

		if (isPending && triggerDelayStopwatch.elapsed() >= kSearchTriggerDelay)
		{
			co_await flushPendingUpdate(currentText);
			continue;
		}

		// Enterで確定(空文字列はキャンセル扱い)
		if (KeyConfig::Down(kButtonStart))
		{
			co_await flushPendingUpdate(currentText);
			noco::CurrentFrame::UnfocusNodeIfFocused(searchBoxNode);
			if (currentText.isEmpty())
			{
				co_return none;
			}
			co_return currentText;
		}

		// Escでキャンセル
		if (KeyConfig::Down(kButtonBack))
		{
			noco::CurrentFrame::UnfocusNodeIfFocused(searchBoxNode);
			co_return none;
		}

		co_await Co::NextFrame();
	}
}

void SearchInputDialog::draw() const
{
	if (!m_canvas)
	{
		return;
	}

	const Transformer2D resetTransform{ Mat3x2::Identity(), Transformer2D::Target::SetLocal };
	m_canvas->draw();
}
