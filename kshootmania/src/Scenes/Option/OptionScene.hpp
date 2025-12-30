#pragma once
#include <CoTaskLib.hpp>
#include "OptionTopMenu.hpp"
#include "OptionMenu.hpp"
#include "OptionKeyConfigMenu.hpp"
#include "ksmaudio/ksmaudio.hpp"

class OptionScene : public Co::UpdaterSceneBase
{
public:
	// TODO: OptionTopMenu::Itemと統一
	enum OptionMenuType : int32
	{
		kDisplaySound = 0,
		kInputJudgment,
		kOther,
		kKeyConfig,

		kOptionMenuTypeEnumCount,
	};

private:
	std::shared_ptr<noco::Canvas> m_canvas;

	OptionTopMenu m_topMenu;

	std::unique_ptr<std::array<OptionMenu, kOptionMenuTypeEnumCount>> m_optionMenus;

	OptionKeyConfigMenu m_keyConfigMenu;

	Optional<OptionMenuType> m_currentOptionMenuIdx = none;

	ksmaudio::Stream m_bgmStream{ "se/option_bgm.ogg", 1.0, false, false, true };

	std::unique_ptr<std::array<OptionMenu, kOptionMenuTypeEnumCount>> makeOptionMenus();

	void refreshOptionMenusForLanguageChange();

	void createSettingItemNodes();

	void refreshSettingItemParams();

public:
	explicit OptionScene();

	virtual void update() override;

	virtual void draw() const override;

	virtual Co::Task<void> fadeIn() override;

	virtual Co::Task<void> fadeOut() override;

	void exitScene();
};
