#pragma once
#include "title_menu.hpp"

class TitleScene : public MyScene
{
private:
	const Texture m_bgTexture;
	TitleMenu m_menu;
	Stopwatch m_exitStopwatch;

public:
	explicit TitleScene(const InitData& initData);

	virtual void update() override;

	virtual void updateFadeIn([[maybe_unused]] double t) { update(); }

	virtual void draw() const override;

	void processMenuItem(TitleMenu::Item item);
};
