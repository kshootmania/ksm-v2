#pragma once
#include <CoTaskLib.hpp>
#include "title_menu.hpp"
#include "title_assets.hpp"
#include "ksmaudio/ksmaudio.hpp"

class TitleScene : public Co::SceneBase
{
private:
	const Texture m_bgTexture{ TextureAsset(TitleTexture::kBG) };

	ksmaudio::Stream m_bgmStream{ "se/title_bgm.ogg", 1.0, false, false, true };
	ksmaudio::Sample m_enterSe{ "se/title_enter.wav" };

	TitleMenu m_menu;

public:
	explicit TitleScene(TitleMenuItem defaultMenuitem);

	virtual Co::Task<Co::SceneFactory> start() override;

	void update();

	virtual void draw() const override;

	virtual Co::Task<void> fadeIn() override;
};
