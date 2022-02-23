#pragma once
#include "ui/simple_menu.hpp"
#include "graphics/texture_atlas.hpp"

class TitleMenu : public ISimpleMenuEventHandler
{
private:
	enum Item : int
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

	double m_easedCursorPos = 0.0;

public:
	TitleMenu();

	void update();

	void draw() const;

	virtual void enterKeyPressed(const MenuEvent& event) override;

	virtual void escKeyPressed(const MenuEvent& event) override;
};
