#include "OptionTopMenu.hpp"
#include "UI/MouseMenuUtils.hpp"

namespace
{
	// クリックイベントのタグとメニュー項目の対応関係
	const Array<std::pair<String, int32>> kClickTagToItemPairs = {
		{ U"onClickDisplaySoundSettings", OptionTopMenu::kDisplaySound },
		{ U"onClickInputJudgmentSettings", OptionTopMenu::kInputJudgment },
		{ U"onClickOtherSettings", OptionTopMenu::kOther },
		{ U"onClickKeyConfiguration", OptionTopMenu::kKeyConfig },
	};
}

OptionTopMenu::OptionTopMenu()
	: m_menu(
		LinearMenu::CreateInfoWithEnumCount{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Vertical,
				.buttonFlags = CursorButtonFlags::kArrowOrLaser | CursorButtonFlags::kMouseWheel,
				.buttonIntervalSec = 0.1,
				.buttonIntervalSecFirst = 0.5,
			},
			.enumCount = kItemEnumCount,
		})
{
}

void OptionTopMenu::update(noco::Canvas* pCanvas)
{
	m_menu.update();

	// クリックされた項目へカーソルを移動して決定
	m_clickDecided = false;
	const Optional<int32> clickedItem = MouseMenuUtils::FiredClickIndex(*pCanvas, kClickTagToItemPairs);
	if (clickedItem.has_value())
	{
		m_menu.setCursor(*clickedItem);
		m_clickDecided = true;
	}
}

bool OptionTopMenu::clickDecided() const
{
	return m_clickDecided;
}

void OptionTopMenu::updateUI(noco::Canvas* pCanvas) const
{
	const auto topMenuNode = pCanvas->findByName(U"TopMenu");
	if (!topMenuNode)
	{
		return;
	}

	const int32 cursorIdx = m_menu.cursor();
	const auto& children = topMenuNode->children();
	for (int32 i = 0; i < static_cast<int32>(children.size()); ++i)
	{
		children[i]->setStyleState(i == cursorIdx ? U"selected" : U"");
	}
}
