#pragma once
#include "music_game/highway_3d_graphics.hpp"

class PlayScene : public MyScene
{
private:
	MusicGame::Highway3DGraphics m_highway3DGraphics;

public:
	explicit PlayScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;
};
