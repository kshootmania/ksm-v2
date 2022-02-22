#pragma once
#include "scene.hpp"
#include "ui/simple_menu.hpp"

class TitleScene : public SceneManager<String>::Scene, public ISimpleMenuEventHandler
{
private:
	Texture m_bgTexture;
	Menu m_menu;

	enum MenuItemType : int
	{
		kStart = 0,
		kOption,
		kInputGate,
		kExit,

		kMenuItemTypeMax,
	};

public:
	static const String kSceneName;

	explicit TitleScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;

	virtual void enterKeyPressed(const MenuEvent& event) override;

	virtual void escKeyPressed(const MenuEvent& event) override;
};
