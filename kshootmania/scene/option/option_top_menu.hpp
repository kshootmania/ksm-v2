#pragma once
#include "ui/menu_helper.hpp"
#include "graphics/texture_atlas.hpp"

class OptionScene;

class OptionTopMenu : public IMenuEventHandler
{
private:
	OptionScene* m_pOptionScene;

	Menu m_menu;

	TextureAtlas m_menuItemTextureAtlas;

	Stopwatch m_stopwatch;

	bool m_active = true;

public:
	enum Item : int32
	{
		kDisplaySound,
		kInputJudgment,
		kOther,
		kKeyConfig,

		kItemMax,
	};

	explicit OptionTopMenu(OptionScene* pOptionScene);

	void update();

	void draw() const;

	virtual void processMenuEvent(const MenuEvent& event) override;

	bool active() const;

	void activate();
};
