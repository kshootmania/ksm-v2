#include "play_scene.hpp"

PlayScene::PlayScene(const InitData& initData)
	: MyScene(initData)
{
	Print << U"Received: {}"_fmt(getData().playSceneArgs.chartFilePath);
}

void PlayScene::update()
{
	m_musicGameGraphics.update(MusicGame::CameraState{});
}

void PlayScene::draw() const
{
	// TODO: Calculate camera state
	m_musicGameGraphics.draw();
}
