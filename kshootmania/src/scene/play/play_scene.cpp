#include "play_scene.hpp"

namespace
{
	MusicGame::GameCreateInfo MakeGameCreateInfo(const PlaySceneArgs& args)
	{
		return
		{
			.chartFilePath = args.chartFilePath,
			.gaugeType = args.gaugeType,
			.assistTickEnabled = ConfigIni::GetBool(ConfigIni::Key::kAssistTick),
		};
	}
}

PlayScene::PlayScene(const InitData& initData)
	: MyScene(initData)
	, m_gameMain(MakeGameCreateInfo(getData().playSceneArgs))
{
	m_gameMain.start();
	ScreenFadeAddon::FadeIn(nullptr, Palette::White);
}

PlayScene::~PlayScene()
{
	m_gameMain.terminate();
}

void PlayScene::update()
{
	m_gameMain.update();

	// Escキーでゲームを中断してリザルト画面へ遷移
	if (!ScreenFadeAddon::IsFadingOut() && KeyConfig::Down(KeyConfig::kBack))
	{
		m_gameMain.lockForExit();

		getData().resultSceneArgs = ResultSceneArgs
		{
			.chartFilePath = FilePath(m_gameMain.chartFilePath()),
			.chartData = m_gameMain.chartData(), // TODO: shared_ptrでコピーを避ける?
			.playResult = m_gameMain.playResult(),
		};

		ScreenFadeAddon::FadeOutToScene(SceneName::kResult);
	}
}

void PlayScene::draw() const
{
	m_gameMain.draw();
}
