#include "menu_helper.hpp"

namespace
{
	Menu MakeMenuCommon(IMenuEventHandler* pDefaultHandler, int32 itemSize, bool cyclic, int32 initialCursorIdx, MenuEventTrigger triggerPrev, MenuEventTrigger triggerNext)
	{
		Menu menu(pDefaultHandler);

		for (int32 i = 0; i < itemSize; ++i)
		{
			MenuItem& menuItem = menu.emplaceMenuItem();

			if (i == 0)
			{
				if (cyclic)
				{
					menuItem.emplaceEventHandler<MoveMenuCursorTo>(triggerPrev, itemSize - 1);
				}
			}
			else
			{
				menuItem.emplaceEventHandler<MoveMenuCursorTo>(triggerPrev, i - 1);
			}

			if (i == itemSize - 1)
			{
				if (cyclic)
				{
					menuItem.emplaceEventHandler<MoveMenuCursorTo>(triggerNext, 0);
				}
			}
			else
			{
				menuItem.emplaceEventHandler<MoveMenuCursorTo>(triggerNext, i + 1);
			}
		}

		menu.moveCursorTo(initialCursorIdx);

		return menu; // <- implicitly moved
	}

	class SharedHandlerForMenuHierarchy : public IMenuEventHandler
	{
	private:
		Menu* const m_pMenu;
		Menu* const m_pOverridenMenu;
		MenuEventTrigger m_enterTrigger;
		MenuEventTrigger m_escapeTrigger;

	public:
		explicit SharedHandlerForMenuHierarchy(Menu* pMenu, Menu* pOverridenMenu, MenuEventTrigger enterTrigger, MenuEventTrigger escapeTrigger)
			: m_pMenu(pMenu)
			, m_pOverridenMenu(pOverridenMenu)
			, m_enterTrigger(enterTrigger)
			, m_escapeTrigger(escapeTrigger)
		{
		}

		virtual ~SharedHandlerForMenuHierarchy() = default;

		virtual void processMenuEvent(const MenuEvent& event) override
		{
			if (event.pMenu == m_pOverridenMenu && event.trigger == m_escapeTrigger)
			{
				m_pMenu->setOverridenMenu(nullptr);
			}
			else if (event.pMenu == m_pMenu && event.trigger == m_enterTrigger)
			{
				m_pMenu->setOverridenMenu(m_pOverridenMenu);
			}
			else
			{
				m_pMenu->publishEventToDefaultEventHandler(event);
			}
		}
	};
}

Menu MenuHelper::MakeVerticalMenu(IMenuEventHandler* pDefaultHandler, int32 itemSize, bool cyclic, int32 initialCursorIdx)
{
	return MakeMenuCommon(pDefaultHandler, itemSize, cyclic, initialCursorIdx, MenuEventTrigger::Up, MenuEventTrigger::Down);
}

Menu MenuHelper::MakeHorizontalMenu(IMenuEventHandler* pDefaultHandler, int32 itemSize, bool cyclic, int32 initialCursorIdx)
{
	return MakeMenuCommon(pDefaultHandler, itemSize, cyclic, initialCursorIdx, MenuEventTrigger::Left, MenuEventTrigger::Right);
}

void MenuHelper::SetChildMenu(Menu* pParentMenu, int32 idx, Menu* pChildMenu, MenuEventTrigger enterTrigger, MenuEventTrigger escapeTrigger)
{
	// HACK: Force to get pointer to share parent's event handler with children
	// (No problem, since parents are supposed to live longer than children)
	SharedHandlerForMenuHierarchy* pSharedHandler = &pParentMenu->emplaceEventHandler<SharedHandlerForMenuHierarchy>(idx, enterTrigger, pParentMenu, pChildMenu, enterTrigger, escapeTrigger);
	pChildMenu->setDefaultEventHandler(pSharedHandler);
}
