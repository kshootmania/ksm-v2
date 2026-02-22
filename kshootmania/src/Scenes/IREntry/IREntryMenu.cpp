#include "IREntryMenu.hpp"

namespace
{
	// STARTボタンを無視する時間の長さ
	constexpr Duration kStartIgnoreDuration = 0.1s;
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
				.buttonFlags = CursorButtonFlags::kArrowOrBTAllOrLaserAll,
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

		if (m_stopwatch.elapsed() >= kStartIgnoreDuration && KeyConfig::Down(kButtonStart))
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
