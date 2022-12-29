#include "play_scene.hpp"

namespace
{
	MusicGame::GameCreateInfo MakeGameCreateInfo(const PlaySceneArgs& args)
	{
		return {
			.chartFilePath = args.chartFilePath,
			.assistTickEnabled = ConfigIni::GetBool(ConfigIni::Key::kAssistTick),
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

	m_hispeedSettingMenu.update();

	// Escキーで楽曲選択画面に戻る
	if (KeyConfig::Down(KeyConfig::kBack))
	{
		changeScene(U"Select", kDefaultTransitionMs);
	}
}

void PlayScene::draw() const
{
	m_gameMain.draw(MusicGame::GameDrawInfo{
		.hispeedSetting = m_hispeedSettingMenu.hispeedSetting(),
	});
}

void PlayScene::updateFadeIn([[maybe_unused]] double t)
{
	update();
}

void PlayScene::updateFadeOut([[maybe_unused]] double t)
{
	update();
}
