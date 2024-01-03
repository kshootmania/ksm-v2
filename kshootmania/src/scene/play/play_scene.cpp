#include "play_scene.hpp"

namespace
{
	constexpr Duration kPlayFinishFadeOutDuration = 2.4s;

	MusicGame::GameCreateInfo MakeGameCreateInfo(const PlaySceneArgs& args)
	{
		return
		{
			.chartFilePath = args.chartFilePath,
			.playOption = args.playOption,
			.assistTickEnabled = ConfigIni::GetBool(ConfigIni::Key::kAssistTick),
		};
	}
}

PlayScene::PlayScene(const InitData& initData)
	: MyScene(initData)
	, m_gameMain(MakeGameCreateInfo(getData().playSceneArgs))
{
	m_gameMain.start();
	ScreenFadeAddon::FadeIn(Palette::White);

	// Playシーンではウィンドウのフォーカスが外れていてもミュートしない
	AutoMuteAddon::SetEnabled(false);
}

PlayScene::~PlayScene()
{
	m_gameMain.terminate();
}

void PlayScene::update()
{
	const auto startFadeOut = m_gameMain.update();

	if (!ScreenFadeAddon::IsFadingOut())
	{
		// 譜面終了時、またはBackボタンでリザルト画面に遷移
		if (startFadeOut)
		{
			getData().resultSceneArgs = ResultSceneArgs
			{
				.chartFilePath = FilePath(m_gameMain.chartFilePath()),
				.chartData = m_gameMain.chartData(), // TODO: shared_ptrでコピーを避ける?
				.playResult = m_gameMain.playResult(),
			};

			m_gameMain.startBGMFadeOut(kPlayFinishFadeOutDuration.count());
			ScreenFadeAddon::FadeOutToScene(SceneName::kResult, kPlayFinishFadeOutDuration);
		}
		else if (KeyConfig::Down(KeyConfig::kBack))
		{
			// Backボタンの場合はスコアが変動しないようロック
			m_gameMain.lockForExit();

			getData().resultSceneArgs = ResultSceneArgs
			{
				.chartFilePath = FilePath(m_gameMain.chartFilePath()),
				.chartData = m_gameMain.chartData(), // TODO: shared_ptrでコピーを避ける?
				.playResult = m_gameMain.playResult(),
			};

			m_gameMain.startBGMFadeOut(ScreenFadeAddon::kDefaultDurationSec);
			ScreenFadeAddon::FadeOutToScene(SceneName::kResult);
		}
	}
}

void PlayScene::draw() const
{
	m_gameMain.draw();
}
