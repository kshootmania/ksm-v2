#pragma once
#include "music_game/graphics/graphics_main.hpp"

class PlayScene : public MyScene
{
private:
	MusicGame::Graphics::GraphicsMain m_musicGameGraphics;

	Stopwatch m_stopwatch;

	MusicGame::Graphics::UpdateInfo m_graphicsUpdateInfo;

public:
	explicit PlayScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;
};
