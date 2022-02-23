#pragma once
#include "ui/simple_menu.hpp"
#include "graphics/texture_atlas.hpp"

class TitleMenu : public ISimpleMenuEventHandler
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

	void menuEnterKeyPressed(const MenuEvent& event);

	void menuEscKeyPressed(const MenuEvent& event);

public:
	TitleMenu();

	void update();

	void draw() const;

	virtual void menuKeyPressed(const MenuEvent& event) override;
};
