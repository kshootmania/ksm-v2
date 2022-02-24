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
	if (m_pOverridenMenu)
	{
		m_pOverridenMenu->update();
		return;
	}

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

void Menu::setDefaultEventHandler(IMenuEventHandler* pDefaultEventHandler)
{
	m_pDefaultEventHandler = pDefaultEventHandler;
}

Menu* Menu::getCurrentTargetOverridenMenu()
{
	if (m_pOverridenMenu)
	{
		return m_pOverridenMenu->getCurrentTargetOverridenMenu();
	}

	return this;
}

bool Menu::isOverriden() const
{
	return m_pOverridenMenu != nullptr;
}

void Menu::setOverridenMenu(Menu* pOverridenMenu)
{
	m_pOverridenMenu = pOverridenMenu;
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
	if (m_pOverridenMenu)
	{
		m_pOverridenMenu->publishEvent(trigger);
		return;
	}

	if (!m_menuItems.empty() && m_menuItems[m_cursorIdx].publishEvent(this, m_cursorIdx, trigger))
	{
		return;
	}

	if (m_pDefaultEventHandler)
	{
		m_pDefaultEventHandler->processMenuEvent({
			.pMenu = this,
			.menuItemIdx = m_cursorIdx,
			.pMenuItem = &m_menuItems[m_cursorIdx],
			.trigger = trigger });
		return;
	}
}

void Menu::publishEventToDefaultEventHandler(const MenuEvent& event)
{
	if (m_pDefaultEventHandler)
	{
		m_pDefaultEventHandler->processMenuEvent(event);
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
