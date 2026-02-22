#pragma once
#include <CoTaskLib.hpp>
#include "TitleMenu.hpp"
#include "TitleAssets.hpp"
#include "ksmaudio/ksmaudio.hpp"

class TitleScene : public Co::SceneBase
{
private:
	std::shared_ptr<ksmaudio::Stream> m_bgmStream = std::make_shared<ksmaudio::Stream>("se/title_bgm.ogg", 1.0, false, false, true);

	std::shared_ptr<noco::Canvas> m_canvas;

	TitleMenu m_menu;

	TitleMenuItem m_selectedMenuItem = TitleMenuItem::kStart;

public:
	explicit TitleScene(TitleMenuItem defaultMenuitem);

	virtual Co::Task<void> start() override;

	void update();

	virtual void draw() const override;

	virtual Co::Task<void> fadeIn() override;

	virtual Co::Task<void> fadeOut() override;
};
