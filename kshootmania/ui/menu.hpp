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
struct MenuEvent;

class Menu
{
private:
	Array<MenuItem> m_menuItems;
	int32 m_cursorIdx = 0;
	IMenuEventHandler* m_pDefaultEventHandler;
	Menu* m_pOverridenMenu = nullptr;
	
public:
	explicit Menu(IMenuEventHandler* pDefaultEventHandler = nullptr);

	MenuItem& emplaceMenuItem();

	template <class MenuEventHandler, class... Args>
	MenuEventHandler& emplaceEventHandler(int32 idx, MenuEventTrigger trigger, Args&&... args);

	void update();

	void moveCursorTo(int32 cursorIdx);

	void publishEvent(MenuEventTrigger trigger);

	void publishEventToDefaultEventHandler(const MenuEvent& event);

	int32 cursorIdx() const;

	void setDefaultEventHandler(IMenuEventHandler* pDefaultEventHandler);

	void setOverridenMenu(Menu* pOverridenMenu);

	Menu* getCurrentTargetOverridenMenu();

	bool isOverriden() const;
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

	virtual void processMenuEvent(const MenuEvent& event) = 0;
};

class MenuItem
{
private:
	std::array<std::unique_ptr<IMenuEventHandler>, kMenuEventTriggerMax> m_eventHandlers;

public:
	MenuItem() = default;

	template <class MenuEventHandler, class... Args>
	MenuEventHandler& emplaceEventHandler(MenuEventTrigger trigger, Args&&... args);

	virtual bool publishEvent(Menu* pMenu, int32 menuItemIdx, MenuEventTrigger trigger);
};

class MoveMenuCursorTo : public IMenuEventHandler
{
private:
	const int32 m_destIdx;

public:
	explicit MoveMenuCursorTo(int32 destIdx);

	virtual ~MoveMenuCursorTo() = default;

	virtual void processMenuEvent(const MenuEvent& event) override;
};


// --- Member function template definitions from here ---

template<class MenuEventHandler, class... Args>
inline MenuEventHandler& Menu::emplaceEventHandler(int32 idx, MenuEventTrigger trigger, Args&&... args)
{
	if (idx < 0 || m_menuItems.size() <= idx)
	{
		throw Error(U"Menu::emplaceEventHandler(): idx(={}) is out of range! (m_menuItems.size()={})"_fmt(idx, m_menuItems.size()));
	}

	return m_menuItems[idx].emplaceEventHandler<MenuEventHandler>(trigger, std::forward<Args>(args)...);
}

template<class MenuEventHandler, class... Args>
inline MenuEventHandler& MenuItem::emplaceEventHandler(MenuEventTrigger trigger, Args&&... args)
{
	int32 triggerInt = static_cast<int32>(trigger);
	if (triggerInt < 0 || m_eventHandlers.size() <= triggerInt)
	{
		throw Error(U"MenuItem::emplaceEventHandler(): triggerInt(={}) is out of range! (m_eventHandlers.size()={})"_fmt(triggerInt, m_eventHandlers.size()));
	}

	MenuEventHandler* const pMenuEventHandler = new MenuEventHandler(std::forward<Args>(args)...);
	m_eventHandlers[triggerInt] = std::unique_ptr<IMenuEventHandler>{ pMenuEventHandler };

	return *pMenuEventHandler;
}
