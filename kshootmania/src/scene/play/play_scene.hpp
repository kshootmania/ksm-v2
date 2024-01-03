#pragma once
#include "music_game/game_main.hpp"

class PlayScene : public MyScene
{
private:
	/// @brief ゲームのメインクラス
	MusicGame::GameMain m_gameMain;

	/// @brief オートプレイかどうか
	MusicGame::IsAutoPlayYN m_isAutoPlay;

public:
	explicit PlayScene(const InitData& initData);

	virtual ~PlayScene();

	virtual void update() override;

	virtual void draw() const override;
};
