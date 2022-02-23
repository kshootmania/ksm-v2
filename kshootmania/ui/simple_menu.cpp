#include "simple_menu.hpp"

namespace
{
	class SimpleMenuEventHandlerPipe : public IMenuEventHandler
	{
	private:
		ISimpleMenuEventHandler* const m_pHandler;

	public:
		explicit SimpleMenuEventHandlerPipe(ISimpleMenuEventHandler* pHandler)
			: m_pHandler(pHandler)
		{
		}

		virtual ~SimpleMenuEventHandlerPipe() = default;

		virtual void run(const MenuEvent& event) override
		{
			if (m_pHandler)
			{
				m_pHandler->menuKeyPressed(event);
			}
		}
	};
	
	Menu MakeSimpleMenuCommon(ISimpleMenuEventHandler* pHandler, int32 itemSize, bool cyclic, MenuEventTrigger triggerPrev, MenuEventTrigger triggerNext)
	{
		Menu menu;

		if (pHandler)
		{
			menu.emplaceDefaultEventHandler<SimpleMenuEventHandlerPipe>(pHandler);
		}

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

		return menu; // <- implicitly moved
	}
}

Menu MakeVerticalSimpleMenu(ISimpleMenuEventHandler* pHandler, int32 itemSize, bool cyclic)
{
	return MakeSimpleMenuCommon(pHandler, itemSize, cyclic, MenuEventTrigger::Up, MenuEventTrigger::Down);
}

Menu MakeHorizontalSimpleMenu(ISimpleMenuEventHandler* pHandler, int32 itemSize, bool cyclic)
{
	return MakeSimpleMenuCommon(pHandler, itemSize, cyclic, MenuEventTrigger::Left, MenuEventTrigger::Right);
}
