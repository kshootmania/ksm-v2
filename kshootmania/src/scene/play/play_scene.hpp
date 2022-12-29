#pragma once
#include "music_game/game_main.hpp"
#include "ui/hispeed_setting_menu.hpp"

class PlayScene : public MyScene
{
private:
	// ゲームのメインクラス
	MusicGame::GameMain m_gameMain;

	// ハイスピード設定のメニュー
	// (HispeedSettingMenuが依存するLinearMenuやConfigIniはMusicGameの外なので、MusicGame::GameMainには入れずにシーン側で持つ)
	HispeedSettingMenu m_hispeedSettingMenu;

public:
	explicit PlayScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;

	virtual void updateFadeIn([[maybe_unused]] double t) override;

	virtual void updateFadeOut([[maybe_unused]] double t) override;
};
