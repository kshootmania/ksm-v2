#pragma once
#include "title_menu.hpp"

class TitleScene : public MyScene
{
private:
	const Texture m_bgTexture;
	TitleMenu m_menu;

public:
	explicit TitleScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;

	void processMenuItem(TitleMenu::Item item);
};
