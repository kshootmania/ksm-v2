#include "play_scene.hpp"

PlayScene::PlayScene(const InitData& initData)
	: MyScene(initData)
{
	Print << U"Received: {}"_fmt(getData().playSceneArgs.chartFilePath);
}

void PlayScene::update()
{
	m_highway3DGraphics.update(MusicGame::CameraState{});
}

void PlayScene::draw() const
{
	// TODO: Calculate camera state
	m_highway3DGraphics.draw();
}
