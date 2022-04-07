#include "play_scene.hpp"

PlayScene::PlayScene(const InitData& initData)
	: MyScene(initData)
{
	Print << U"Received: {}"_fmt(getData().playSceneArgs.chartFilePath);
}

void PlayScene::update()
{
}

void PlayScene::draw() const
{
}
