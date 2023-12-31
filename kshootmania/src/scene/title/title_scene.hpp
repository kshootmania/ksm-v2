#pragma once
#include "title_menu.hpp"
#include "ksmaudio/ksmaudio.hpp"

class TitleScene : public MyScene
{
private:
	const Texture m_bgTexture;
	TitleMenu m_menu;
	ksmaudio::Stream m_bgmStream{ "se/title_bgm.ogg", 1.0, false, false, true };
	ksmaudio::Sample m_enterSe{ "se/title_enter.wav" };

public:
	explicit TitleScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;

	void processMenuItem(TitleMenu::Item item);
};
