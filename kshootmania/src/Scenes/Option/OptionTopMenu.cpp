#include "OptionTopMenu.hpp"

OptionTopMenu::OptionTopMenu()
	: m_menu(
		LinearMenu::CreateInfoWithEnumCount{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Vertical,
				.buttonFlags = CursorButtonFlags::kArrowOrLaser,
				.buttonIntervalSec = 0.1,
				.buttonIntervalSecFirst = 0.5,
			},
			.enumCount = kItemEnumCount,
		})
{
}

void OptionTopMenu::update()
{
	m_menu.update();
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
