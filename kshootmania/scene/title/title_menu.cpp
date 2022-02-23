#include "title_menu.hpp"

TitleMenu::TitleMenu(TitleScene* pTitleScene)
	: m_pTitleScene(pTitleScene)
	, m_menu(MakeVerticalSimpleMenu(this, kItemMax))
{
}

void TitleMenu::update()
{
	m_menu.update();
}

void TitleMenu::enterKeyPressed(const MenuEvent& event)
{
	switch (event.menuItemIdx)
	{
	case kStart:
		break;

	case kOption:
		break;

	case kInputGate:
		break;

	case kExit:
		break;
	}
}

void TitleMenu::escKeyPressed(const MenuEvent&)
{
	m_menu.moveCursorTo(kExit);
}
