#include "menu.hpp"

MenuItem& Menu::emplaceMenuItem()
{
	return m_menuItems.emplace_back();
}

void Menu::update()
{
	if (KeyEnter.down())
	{
		runEvent(MenuEventTrigger::Enter);
	}

	if (KeyBackspace.down())
	{
		runEvent(MenuEventTrigger::Backspace);
	}

	if (KeyUp.down())
	{
		runEvent(MenuEventTrigger::Up);
	}

	if (KeyDown.down())
	{
		runEvent(MenuEventTrigger::Down);
	}

	if (KeyLeft.down())
	{
		runEvent(MenuEventTrigger::Left);
	}

	if (KeyRight.down())
	{
		runEvent(MenuEventTrigger::Right);
	}

	if (KeyEscape.down())
	{
		runEvent(MenuEventTrigger::Esc);
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

void Menu::runEvent(MenuEventTrigger trigger)
{
	const bool handlerExists = m_menuItems[m_cursorIdx].runEvent(this, m_cursorIdx, trigger);
	if (!handlerExists && m_defaultEventHandler)
	{
		m_defaultEventHandler->run({
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

bool MenuItem::runEvent(Menu* pMenu, int32 menuItemIdx, MenuEventTrigger trigger)
{
	int32 triggerInt = static_cast<int32>(trigger);
	if (triggerInt < 0 || m_eventHandlers.size() <= triggerInt)
	{
		throw Error(U"MenuItem::runEvent(): triggerInt(={}) is out of range! (m_eventHandlers.size()={})"_fmt(triggerInt, m_eventHandlers.size()));
	}

	if (m_eventHandlers[triggerInt])
	{
		m_eventHandlers[triggerInt]->run({
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

void MoveMenuCursorTo::run(const MenuEvent& event)
{
	event.pMenu->moveCursorTo(m_destIdx);
}
