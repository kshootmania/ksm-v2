#pragma once
#include "ui/menu_helper.hpp"
#include "graphics/texture_atlas.hpp"

class TitleMenu : public IMenuEventHandler
{
private:
	enum Item : int32
	{
		kStart = 0,
		kOption,
		kInputGate,
		kExit,

		kItemMax,
	};

	Menu m_menu;

	TextureAtlas m_menuItemTextureAtlas;

	Texture m_menuCursorTexture;

	Stopwatch m_stopwatch;

	Stopwatch m_exitStopwatch;

	double m_easedCursorPos = 0.0;

	void enterKeyPressed(const MenuEvent& event);

	void escKeyPressed(const MenuEvent& event);

public:
	TitleMenu();

	void update();

	void draw() const;

	virtual void processMenuEvent(const MenuEvent& event) override;
};
