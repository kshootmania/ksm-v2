#pragma once
#include "menu.hpp"

class ISimpleMenuEventHandler
{
public:
	virtual ~ISimpleMenuEventHandler() = default;

	virtual void enterKeyPressed(const MenuEvent& event) = 0;

	virtual void escKeyPressed(const MenuEvent& event) = 0;
};

Menu MakeVerticalSimpleMenu(ISimpleMenuEventHandler* pHandler, int itemSize, bool cyclic = false);

Menu MakeHorizontalSimpleMenu(ISimpleMenuEventHandler* pHandler, int itemSize, bool cyclic = false);
