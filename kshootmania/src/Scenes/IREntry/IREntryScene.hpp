#pragma once
#include <CoTaskLib.hpp>
#include "IREntryMenu.hpp"
#include "ksmaudio/ksmaudio.hpp"

class IREntryScene : public Co::SceneBase
{
private:
	std::shared_ptr<ksmaudio::Stream> m_bgmStream;

	std::shared_ptr<noco::Canvas> m_canvas;

	IREntryMenu m_menu;

	bool m_goBackToTitle = false;

public:
	explicit IREntryScene(std::shared_ptr<ksmaudio::Stream> bgmStream);

	virtual Co::Task<void> start() override;

	void update();

	virtual void draw() const override;

	virtual Co::Task<void> fadeIn() override;

	virtual Co::Task<void> fadeOut() override;
};
