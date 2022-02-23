#include "menu.hpp"

Menu::Menu(IMenuEventHandler* pDefaultEventHandler)
	: m_pDefaultEventHandler(pDefaultEventHandler)
{
}

MenuItem& Menu::emplaceMenuItem()
{
	return m_menuItems.emplace_back();
}

void Menu::update()
{
	if (KeyEnter.down())
	{
		publishEvent(MenuEventTrigger::Enter);
	}

	if (KeyBackspace.down())
	{
		publishEvent(MenuEventTrigger::Backspace);
	}

	if (KeyUp.down())
	{
		publishEvent(MenuEventTrigger::Up);
	}

	if (KeyDown.down())
	{
		publishEvent(MenuEventTrigger::Down);
	}

	if (KeyLeft.down())
	{
		publishEvent(MenuEventTrigger::Left);
	}

	if (KeyRight.down())
	{
		publishEvent(MenuEventTrigger::Right);
	}

	if (KeyEscape.down())
	{
		publishEvent(MenuEventTrigger::Esc);
	}
}

void Menu::moveCursorTo(int32 cursorIdx)
{
	if (cursorIdx < 0 || m_menuItems.size() <= cursorIdx)
	{
		throw Error(U"Menu::moveCursorTo(): cursorIdx(={}) is out of range! (m_menuItems.size()={})"_fmt(cursorIdx, m_menuItems.size()));
	}
	m_cursorIdx = cursorIdx;
}

void Menu::publishEvent(MenuEventTrigger trigger)
{
	const bool eventProcessed = m_menuItems[m_cursorIdx].publishEvent(this, m_cursorIdx, trigger);
	if (!eventProcessed && m_pDefaultEventHandler)
	{
		m_pDefaultEventHandler->processMenuEvent({
			.pMenu = this,
			.menuItemIdx = m_cursorIdx,
			.pMenuItem = &m_menuItems[m_cursorIdx],
			.trigger = trigger });
	}
}

int32 Menu::cursorIdx() const
{
	return m_cursorIdx;
}

bool MenuItem::publishEvent(Menu* pMenu, int32 menuItemIdx, MenuEventTrigger trigger)
{
	int32 triggerInt = static_cast<int32>(trigger);
	if (triggerInt < 0 || m_eventHandlers.size() <= triggerInt)
	{
		throw Error(U"MenuItem::publishEvent(): triggerInt(={}) is out of range! (m_eventHandlers.size()={})"_fmt(triggerInt, m_eventHandlers.size()));
	}

	if (m_eventHandlers[triggerInt])
	{
		m_eventHandlers[triggerInt]->processMenuEvent({
			.pMenu = pMenu,
			.menuItemIdx = menuItemIdx,
			.pMenuItem = this,
			.trigger = trigger });
		return true;
	}

	return false;
}

MoveMenuCursorTo::MoveMenuCursorTo(int32 destIdx)
	: m_destIdx(destIdx)
{
}

void MoveMenuCursorTo::processMenuEvent(const MenuEvent& event)
{
	event.pMenu->moveCursorTo(m_destIdx);
}
