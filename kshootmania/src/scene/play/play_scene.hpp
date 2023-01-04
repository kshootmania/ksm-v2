#pragma once
#include "music_game/game_main.hpp"

class PlayScene : public MyScene
{
private:
	// ゲームのメインクラス
	MusicGame::GameMain m_gameMain;

public:
	explicit PlayScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;

	virtual void updateFadeIn([[maybe_unused]] double t) override;

	virtual void updateFadeOut([[maybe_unused]] double t) override;
};
