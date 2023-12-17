#pragma once
#include "music_game/game_main.hpp"

class PlayScene : public MyScene
{
private:
	// ゲームのメインクラス
	MusicGame::GameMain m_gameMain;

public:
	explicit PlayScene(const InitData& initData);

	virtual ~PlayScene();

	virtual void update() override;

	virtual void draw() const override;
};
