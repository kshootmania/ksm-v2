#include "play_scene.hpp"
#include "scene/select/select_scene.hpp"
#include "scene/result/result_scene.hpp"

namespace
{
	constexpr Duration kFadeDuration = 0.6s;

	constexpr Duration kPlayFinishFadeOutDuration = 2.4s;

	MusicGame::GameCreateInfo MakeGameCreateInfo(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay)
	{
		return
		{
			.chartFilePath = FilePath{ chartFilePath },
			.playOption = MusicGame::PlayOption
			{
				.isAutoPlay = isAutoPlay,
				// TODO: 他のオプション
			},
			.assistTickEnabled = ConfigIni::GetBool(ConfigIni::Key::kAssistTick),
		};
	}
}

PlayScene::PlayScene(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay)
	: m_gameMain(MakeGameCreateInfo(chartFilePath, isAutoPlay))
	, m_isAutoPlay(isAutoPlay)
	, m_fadeOutDuration(kFadeDuration)
{
	m_gameMain.start();

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

	// 譜面終了時、またはBackボタンでリザルト画面に遷移
	if (startFadeOut)
	{
		m_fadeOutDuration = kPlayFinishFadeOutDuration;

		if (m_isAutoPlay)
		{
			requestNextScene<SelectScene>();
		}
		else
		{
			const ResultSceneArgs args =
			{
				.chartFilePath = FilePath(m_gameMain.chartFilePath()),
				.chartData = m_gameMain.chartData(), // TODO: shared_ptrでコピーを避ける?
				.playResult = m_gameMain.playResult(),
			};
			requestNextScene<ResultScene>(args);
		}
	}
	else if (KeyConfig::Down(KeyConfig::kBack))
	{
		m_fadeOutDuration = 0s;

		// Backボタンの場合はスコアが変動しないようロック
		m_gameMain.lockForExit();

		if (m_isAutoPlay)
		{
			requestNextScene<SelectScene>();
		}
		else
		{
			const ResultSceneArgs args =
			{
				.chartFilePath = FilePath(m_gameMain.chartFilePath()),
				.chartData = m_gameMain.chartData(), // TODO: shared_ptrでコピーを避ける?
				.playResult = m_gameMain.playResult(),
			};
			requestNextScene<ResultScene>(args);
		}
	}
}

void PlayScene::draw() const
{
	m_gameMain.draw();
}

inline Co::Task<void> PlayScene::fadeIn()
{
	co_await Co::SimpleFadeIn(kFadeDuration);
}

Co::Task<void> PlayScene::fadeOut()
{
	// 再生時間の進行を止めないためにフェードアウト中もGameMainのupdateは実行
	const Co::ScopedUpdater updater([this] { m_gameMain.update(); });

	m_gameMain.startBGMFadeOut(m_fadeOutDuration);
	co_await Co::SimpleFadeOut(m_fadeOutDuration);
}
