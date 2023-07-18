#include "play_scene.hpp"

namespace
{
	MusicGame::GameCreateInfo MakeGameCreateInfo(const PlaySceneArgs& args)
	{
		return
		{
			.chartFilePath = args.chartFilePath,
			.assistTickEnabled = ConfigIni::GetBool(ConfigIni::Key::kAssistTick),
		};
	}
}

PlayScene::PlayScene(const InitData& initData)
	: MyScene(initData)
	, m_gameMain(MakeGameCreateInfo(getData().playSceneArgs))
{
	m_gameMain.start();
}

PlayScene::~PlayScene()
{
	m_gameMain.terminate();
}

void PlayScene::update()
{
	m_gameMain.update();

	// Escキーでゲームを中断してリザルト画面へ遷移
	if (KeyConfig::Down(KeyConfig::kBack))
	{
		getData().resultSceneArgs =
		{
			.chartData = m_gameMain.chartData(), // TODO: shared_ptrでコピーを避ける?
			.playResult = m_gameMain.playResult(),
		};

		changeScene(SceneName::kResult, kDefaultTransitionMs);
	}
}

void PlayScene::draw() const
{
	m_gameMain.draw();
}

void PlayScene::updateFadeIn([[maybe_unused]] double t)
{
	update();
}

void PlayScene::updateFadeOut([[maybe_unused]] double t)
{
	update();
}
