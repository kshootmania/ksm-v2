#include "IREntryMenu.hpp"
#include "UI/MouseMenuUtils.hpp"

namespace
{
	// STARTボタンを無視する時間の長さ
	constexpr Duration kStartIgnoreDuration = 0.1s;

	// クリックイベントのタグとメニュー項目の対応関係
	const Array<std::pair<String, int32>> kClickTagToMenuItemPairs = {
		{ U"onClickSignIn", IREntryMenu::kSignIn },
		{ U"onClickDontUse", IREntryMenu::kDontUse },
	};
}

void IREntryMenu::refreshCanvasCursorIndex()
{
	m_canvas->setParamValue(U"cursorIndex", m_menu.cursor());
}

IREntryMenu::IREntryMenu(const std::shared_ptr<noco::Canvas>& canvas)
	: m_menu(
		LinearMenu::CreateInfoWithEnumCount
		{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Vertical,
				.buttonFlags = CursorButtonFlags::kArrowOrBTAllOrLaserAll | CursorButtonFlags::kMouseWheel,
			},
			.enumCount = MenuItem::kItemEnumCount,
			.cyclic = IsCyclicMenuYN::No,
		})
	, m_canvas(canvas)
{
	refreshCanvasCursorIndex();
}

void IREntryMenu::update()
{
	const auto beforeCursor = m_menu.cursor();

	if (!m_isAlreadySelected)
	{
		m_menu.update();

		// クリックされた項目がカーソル位置と同じ場合は決定、異なる場合はカーソル移動
		const Optional<int32> clickedItem = MouseMenuUtils::FiredClickIndex(*m_canvas, kClickTagToMenuItemPairs);
		const bool clickDecided = clickedItem.has_value() && *clickedItem == m_menu.cursor();
		if (clickedItem.has_value() && !clickDecided)
		{
			m_menu.setCursor(*clickedItem);
		}

		if (m_stopwatch.elapsed() >= kStartIgnoreDuration && (KeyConfig::Down(kButtonStart) || clickDecided))
		{
			const auto selectedItem = m_menu.cursorAs<MenuItem>();
			m_selectedMenuItemSource.requestFinish(selectedItem);
			m_isAlreadySelected = true;
		}
		else if (KeyConfig::Down(kButtonBack))
		{
			m_selectedMenuItemSource.requestFinish(none);
			m_isAlreadySelected = true;
		}
	}

	if (m_menu.cursor() != beforeCursor)
	{
		m_selectSe.play();
		refreshCanvasCursorIndex();
	}
}
