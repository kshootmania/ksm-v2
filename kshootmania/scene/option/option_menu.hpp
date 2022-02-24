#pragma once
#include "ui/menu_helper.hpp"
#include "graphics/texture_atlas.hpp"

class OptionScene;

class OptionMenu : public IMenuEventHandler
{
private:
	OptionScene* m_pOptionScene;

	Menu m_menu;

	Menu m_subMenu;//FIXME

	TextureAtlas m_menuItemTextureAtlas;

	Stopwatch m_stopwatch;

public:
	enum Item : int32
	{
		kDisplaySound,
		kInputJudgment,
		kOther,
		kKeyConfig,

		kItemMax,
	};

	explicit OptionMenu(OptionScene* pOptionScene);

	void update();

	void draw() const;

	virtual void processMenuEvent(const MenuEvent& event) override;

	Optional<Item> currentActiveMenu() const;
};
