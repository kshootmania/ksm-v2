#pragma once
#include <CoTaskLib.hpp>
#include "title_menu.hpp"
#include "title_assets.hpp"
#include "ksmaudio/ksmaudio.hpp"

class TitleScene : public Co::SequenceBase<TitleMenuItem>
{
private:
	const Texture m_bgTexture{ TextureAsset(TitleTexture::kBG) };
	TitleMenu m_menu;
	ksmaudio::Stream m_bgmStream{ "se/title_bgm.ogg", 1.0, false, false, true };
	ksmaudio::Sample m_enterSe{ "se/title_enter.wav" };

public:
	TitleScene() = default;

	virtual Co::Task<TitleMenuItem> start() override;

	virtual void draw() const override;
};
