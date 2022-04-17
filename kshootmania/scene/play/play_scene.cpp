#include "play_scene.hpp"

namespace
{
	MusicGame::GameCreateInfo MakeGameCreateInfo(const PlaySceneArgs& args)
	{
		return {
			.chartFilePath = args.chartFilePath,
			.enableAssistTick = ConfigIni::GetBool(ConfigIni::Key::kAssistTick),
		};
	}
}

PlayScene::PlayScene(const InitData& initData)
	: MyScene(initData)
	, m_gameMain(MakeGameCreateInfo(getData().playSceneArgs))
{
}

void PlayScene::update()
{
	m_gameMain.update();
}

void PlayScene::draw() const
{
	m_gameMain.draw();
}
