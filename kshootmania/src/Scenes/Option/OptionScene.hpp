#pragma once
#include <CoTaskLib.hpp>
#include "OptionTopMenu.hpp"
#include "OptionMenu.hpp"
#include "OptionKeyConfigMenu.hpp"
#include "UI/ClickHoldRepeat.hpp"
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

	// 設定項目の左右端の押下状態(クリックまたは長押しでの値変更用)
	bool m_valueLeftPressed = false;
	bool m_valueRightPressed = false;
	ClickHoldRepeat m_valueLeftRepeat{ 0.1, 0.5 };
	ClickHoldRepeat m_valueRightRepeat{ 0.1, 0.5 };

	std::unique_ptr<std::array<OptionMenu, kOptionMenuTypeEnumCount>> makeOptionMenus();

	OptionMenu::MouseInput settingMenuMouseInput();

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
