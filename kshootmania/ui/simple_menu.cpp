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
			if (!m_pHandler)
			{
				return;
			}

			switch (event.trigger)
			{
			case MenuEventTrigger::Enter:
				m_pHandler->enterKeyPressed(event);
				break;

			case MenuEventTrigger::Esc:
				m_pHandler->escKeyPressed(event);
				break;

			default:
				break;
			}
		}
	};
	
	Menu MakeSimpleMenuCommon(ISimpleMenuEventHandler* pHandler, int itemSize, bool cyclic, MenuEventTrigger triggerPrev, MenuEventTrigger triggerNext)
	{
		Menu menu;
		for (int i = 0; i < itemSize; ++i)
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

			if (pHandler)
			{
				menuItem.emplaceEventHandler<SimpleMenuEventHandlerPipe>(MenuEventTrigger::Enter, pHandler);
				menuItem.emplaceEventHandler<SimpleMenuEventHandlerPipe>(MenuEventTrigger::Esc, pHandler);
			}
		}
		return menu; // <- implicitly moved
	}
}

Menu MakeVerticalSimpleMenu(ISimpleMenuEventHandler* pHandler, int itemSize, bool cyclic)
{
	return MakeSimpleMenuCommon(pHandler, itemSize, cyclic, MenuEventTrigger::Up, MenuEventTrigger::Down);
}

Menu MakeHorizontalSimpleMenu(ISimpleMenuEventHandler* pHandler, int itemSize, bool cyclic)
{
	return MakeSimpleMenuCommon(pHandler, itemSize, cyclic, MenuEventTrigger::Left, MenuEventTrigger::Right);
}
