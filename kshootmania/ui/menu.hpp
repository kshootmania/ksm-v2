#pragma once

enum class MenuEventTrigger : int32
{
	Enter = 0,
	Esc,
	Up,
	Down,
	Left,
	Right,
	Backspace,

	Max,
};

constexpr int32 kMenuEventTriggerMax = static_cast<int32>(MenuEventTrigger::Max);

class MenuItem;
class IMenuEventHandler;

class Menu
{
private:
	Array<MenuItem> m_menuItems;
	int32 m_cursorIdx = 0;
	std::unique_ptr<IMenuEventHandler> m_defaultEventHandler;
	
public:
	Menu() = default;

	MenuItem& emplaceMenuItem();

	template <class MenuEventHandler, class... Args>
	void emplaceDefaultEventHandler(Args&&... args);

	void update();

	void moveCursorTo(int32 cursorIdx);

	void runEvent(MenuEventTrigger trigger);

	int32 cursorIdx() const;
};

struct MenuEvent
{
	Menu* pMenu;

	int32 menuItemIdx;

	MenuItem* pMenuItem;

	MenuEventTrigger trigger;
};

class IMenuEventHandler
{
public:
	virtual ~IMenuEventHandler() = default;

	virtual void run(const MenuEvent& event) = 0;
};

class MenuItem
{
private:
	std::array<std::unique_ptr<IMenuEventHandler>, kMenuEventTriggerMax> m_eventHandlers;

public:
	MenuItem() = default;

	template <class MenuEventHandler, class... Args>
	MenuItem& emplaceEventHandler(MenuEventTrigger trigger, Args&&... args);

	virtual bool runEvent(Menu* pMenu, int32 menuItemIdx, MenuEventTrigger trigger);
};

class MoveMenuCursorTo : public IMenuEventHandler
{
private:
	const int32 m_destIdx;

public:
	explicit MoveMenuCursorTo(int32 destIdx);

	virtual ~MoveMenuCursorTo() = default;

	virtual void run(const MenuEvent& event) override;
};


// --- Member function template definitions from here ---

template<class MenuEventHandler, class... Args>
inline void Menu::emplaceDefaultEventHandler(Args&&... args)
{
	m_defaultEventHandler = std::make_unique<MenuEventHandler>(std::forward<Args>(args)...);
}


template<class MenuEventHandler, class... Args>
inline MenuItem& MenuItem::emplaceEventHandler(MenuEventTrigger trigger, Args&&... args)
{
	int32 triggerInt = static_cast<int32>(trigger);
	if (triggerInt < 0 || m_eventHandlers.size() <= triggerInt)
	{
		throw Error(U"MenuItem::emplaceEventHandler(): triggerInt(={}) is out of range! (m_eventHandlers.size()={})"_fmt(triggerInt, m_eventHandlers.size()));
	}

	m_eventHandlers[triggerInt] = std::make_unique<MenuEventHandler>(std::forward<Args>(args)...);

	return *this;
}
