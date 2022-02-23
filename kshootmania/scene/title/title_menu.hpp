#pragma once
#include "title_scene.hpp"
#include "ui/simple_menu.hpp"

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
	TitleScene* m_pTitleScene;
	Menu m_menu;

public:
	explicit TitleMenu(TitleScene* pTitleScene);

	void update();

	virtual void enterKeyPressed(const MenuEvent& event) override;

	virtual void escKeyPressed(const MenuEvent& event) override;
};
