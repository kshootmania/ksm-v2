#pragma once
#include "menu.hpp"

namespace MenuHelper
{
	Menu MakeVerticalMenu(IMenuEventHandler* pDefaultHandler, int32 itemSize, bool cyclic = false, int32 initialCursorIdx = 0);

	Menu MakeHorizontalMenu(IMenuEventHandler* pDefaultHandler, int32 itemSize, bool cyclic = false, int32 initialCursorIdx = 0);
}
