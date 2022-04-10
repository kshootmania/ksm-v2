#pragma once
#include "music_game/graphics/graphics_main.hpp"
#include "ksh/util/timing_utils.hpp"

class PlayScene : public MyScene
{
private:
	const ksh::ChartData m_chartData;
	const ksh::TimingCache m_timingCache;

	MusicGame::Graphics::GraphicsMain m_musicGameGraphics;

	Stopwatch m_stopwatch;

	MusicGame::Graphics::UpdateInfo m_graphicsUpdateInfo;

public:
	explicit PlayScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;
};
