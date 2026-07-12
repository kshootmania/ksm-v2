#include "TitleMenu.hpp"
#include "UI/MouseMenuUtils.hpp"

namespace
{
	// STARTボタンを無視する時間の長さ
	constexpr Duration kStartIgnoreDuration = 0.1s;

	// クリックイベントのタグとメニュー項目の対応関係
	const Array<std::pair<String, int32>> kClickTagToMenuItemPairs = {
		{ U"onClickStart", TitleMenuItem::kStart },
		{ U"onClickOption", TitleMenuItem::kOption },
		{ U"onClickInputGate", TitleMenuItem::kInputGate },
		{ U"onClickExit", TitleMenuItem::kExit },
	};
}

void TitleMenu::refreshCanvasMenuCursor()
{
	m_titleSceneCanvas->setParamValue(U"menuCursor", m_menu.cursor());
}

TitleMenu::TitleMenu(TitleMenuItem defaultMenuitem, const std::shared_ptr<noco::Canvas>& titleSceneCanvas)
	: m_menu(
		LinearMenu::CreateInfoWithEnumCount
		{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Vertical,
				.buttonFlags = CursorButtonFlags::kArrowOrBTAllOrLaserAll | CursorButtonFlags::kMouseWheel,
			},
			.enumCount = TitleMenuItem::kItemEnumCount,
			.cyclic = IsCyclicMenuYN::No,
			.defaultCursor = defaultMenuitem,
		})
	, m_titleSceneCanvas(titleSceneCanvas)
{
	refreshCanvasMenuCursor();
}

void TitleMenu::update()
{
	// カーソル変化検出用
	// (LinearMenu::deltaCursor() != 0 だとsetCursorでの直接指定による変化を検出できないため、cursor値の比較を利用している)
	const auto beforeCursor = m_menu.cursor();

	const bool backPressed = !m_isAlreadySelected && KeyConfig::Down(kButtonBack);
	bool clickDecided = false;
	if (!m_isAlreadySelected && !backPressed)
	{
		m_menu.update();

		// クリックされた項目へカーソルを移動して決定
		const Optional<int32> clickedItem = MouseMenuUtils::FiredClickIndex(*m_titleSceneCanvas, kClickTagToMenuItemPairs);
		clickDecided = clickedItem.has_value();
		if (clickedItem.has_value())
		{
			m_menu.setCursor(*clickedItem);
		}

		if (m_stopwatch.elapsed() >= kStartIgnoreDuration && (KeyConfig::Down(kButtonStart) || clickDecided))
		{
			const auto selectedItem = m_menu.cursorAs<TitleMenuItem>();
			m_selectedMenuItemSource.requestFinish(selectedItem);
			m_isAlreadySelected = true;
		}
	}

	// BackボタンでEXITへフォーカス
	if (!m_isAlreadySelected && backPressed)
	{
		m_menu.setCursor(kExit);
	}

	if (m_menu.cursor() != beforeCursor)
	{
		// カーソル位置が変化した場合はCanvasへ反映し、クリック決定時以外は効果音を鳴らす
		if (!clickDecided)
		{
			m_selectSe.play();
		}
		refreshCanvasMenuCursor();
	}
}
