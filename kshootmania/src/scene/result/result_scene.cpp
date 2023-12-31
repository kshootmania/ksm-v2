#include "result_scene.hpp"
#include "high_score/ksc_io.hpp"

ResultScene::ResultScene(const InitData& initData)
	: MyScene(initData)
	, m_chartData(getData().resultSceneArgs.chartData)
	, m_playResult(getData().resultSceneArgs.playResult)
	, m_resultPanel(getData().resultSceneArgs.chartFilePath, m_chartData, m_playResult)
{
	// TODO(alphaまで): 実際の設定を反映
	const KscKey condition
	{
		.gaugeType = GaugeType::kNormalGauge,
		.turnMode = TurnMode::kNormal,
		.btPlayMode = JudgmentPlayMode::kOn,
		.fxPlayMode = JudgmentPlayMode::kOn,
		.laserPlayMode = JudgmentPlayMode::kOn,
	};
	const FilePathView chartFilePath = getData().resultSceneArgs.chartFilePath;
	KscIo::WriteHighScoreInfo(chartFilePath, m_playResult, condition);

	m_bgmStream.play();

	ScreenFadeAddon::FadeIn(Palette::White);
	AutoMuteAddon::SetEnabled(true);
}

void ResultScene::update()
{
	if (ScreenFadeAddon::IsFadingOut())
	{
		return;
	}

	// StartボタンまたはBackボタンで楽曲選択画面に戻る
	if (KeyConfig::Down(KeyConfig::kStart) || KeyConfig::Down(KeyConfig::kBack))
	{
		m_bgmStream.setFadeOut(ScreenFadeAddon::kDefaultDurationSec);
		ScreenFadeAddon::FadeOutToScene(SceneName::kSelect);
		return;
	}
}

void ResultScene::draw() const
{
	FitToHeight(m_bgTexture).drawAt(Scene::Center());

	m_resultPanel.draw();
}
