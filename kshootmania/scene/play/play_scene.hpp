#pragma once
#include "music_game/music_game_graphics.hpp"

class PlayScene : public MyScene
{
private:
	MusicGame::MusicGameGraphics m_musicGameGraphics;

public:
	explicit PlayScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;
};
