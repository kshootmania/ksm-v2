﻿#include "result_scene.hpp"
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
}

void ResultScene::update()
{
	// StartボタンまたはBackボタンで楽曲選択画面に戻る
	if (KeyConfig::Down(KeyConfig::kStart) || KeyConfig::Down(KeyConfig::kBack))
	{
		changeScene(SceneName::kSelect, kDefaultTransitionMs);
		m_bgmStream.setFadeOut(kDefaultTransitionMs / 1000.0);
	}
}

void ResultScene::updateFadeIn([[maybe_unused]] double t)
{
	update();
}

void ResultScene::draw() const
{
	FitToHeight(m_bgTexture).drawAt(Scene::Center());

	m_resultPanel.draw();
}

void ResultScene::drawFadeIn(double t) const
{
	draw();

	// 画面全体を白色でフェード
	Scene::Rect().draw(ColorF(1.0, 1.0 - t));
}
