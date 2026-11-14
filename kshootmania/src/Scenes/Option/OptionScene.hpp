#pragma once
#include <CoTaskLib.hpp>
#include "OptionTopMenu.hpp"
#include "OptionMenu.hpp"
#include "OptionKeyConfigMenu.hpp"
#include "Scenes/Common/CommonOverlay.hpp"
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

	CommonOverlay m_commonOverlay;

	Optional<OptionMenuType> m_currentOptionMenuIdx = none;

	// 設定項目のノード(見出しは含まない)
	Array<std::shared_ptr<noco::Node>> m_settingItemNodes;

	// 言語変更はメニューのコールバックから呼ばれるため、メニューの再生成はupdate内で行う
	bool m_languageChangeRequested = false;

	// カーソル位置に合わせて設定項目をスクロール中かどうか
	bool m_isSettingMenuScrollFollowingCursor = false;

	double m_settingMenuScrollVelocity = 0.0;

	ksmaudio::Stream m_bgmStream{ "se/option_bgm.ogg", 1.0, false, false, true };

	std::unique_ptr<std::array<OptionMenu, kOptionMenuTypeEnumCount>> makeOptionMenus();

	OptionMenu::MouseInput settingMenuMouseInput();

	void refreshOptionMenusForLanguageChange();

	void createSettingItemNodes();

	void refreshSettingItemParams();

	/// @brief カーソル位置の項目が中央に来る設定項目のスクロール位置を取得する
	/// @return スクロール位置(設定項目がない場合はnone)
	[[nodiscard]]
	Optional<double> settingMenuScrollTargetY() const;

	/// @brief 設定項目のスクロール位置をカーソル位置へ即座に合わせる
	void resetSettingMenuScroll();

	/// @brief 設定項目のスクロール位置をカーソル位置へ滑らかに近づける
	void updateSettingMenuScroll();

public:
	explicit OptionScene();

	virtual void update() override;

	virtual void draw() const override;

	virtual Co::Task<void> fadeIn() override;

	virtual Co::Task<void> fadeOut() override;

	void exitScene();
};
