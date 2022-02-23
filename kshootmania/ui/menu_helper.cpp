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
}

Menu MenuHelper::MakeVerticalMenu(IMenuEventHandler* pDefaultHandler, int32 itemSize, bool cyclic, int32 initialCursorIdx)
{
	return MakeMenuCommon(pDefaultHandler, itemSize, cyclic, initialCursorIdx, MenuEventTrigger::Up, MenuEventTrigger::Down);
}

Menu MenuHelper::MakeHorizontalMenu(IMenuEventHandler* pDefaultHandler, int32 itemSize, bool cyclic, int32 initialCursorIdx)
{
	return MakeMenuCommon(pDefaultHandler, itemSize, cyclic, initialCursorIdx, MenuEventTrigger::Left, MenuEventTrigger::Right);
}
