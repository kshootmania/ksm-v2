#include "result_scene.hpp"
#include "scene/select/select_scene.hpp"
#include "high_score/ksc_io.hpp"

namespace
{
	constexpr Duration kFadeDuration = 0.6s;
}

ResultScene::ResultScene(const ResultSceneArgs& args)
	: m_chartData(args.chartData)
	, m_playResult(args.playResult)
	, m_resultPanel(args.chartFilePath, m_chartData, m_playResult)
{
	if (!m_playResult.playOption.isAutoPlay) // オートプレイの場合はスコアを保存しない(オートプレイではリザルト画面を出さないので不要だが一応チェックはする)
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
		const FilePathView chartFilePath = args.chartFilePath;
		KscIo::WriteHighScoreInfo(chartFilePath, m_playResult, condition);
	}

	m_bgmStream.play();

	AutoMuteAddon::SetEnabled(true);
}

void ResultScene::update()
{
	// StartボタンまたはBackボタンで楽曲選択画面に戻る
	if (KeyConfig::Down(KeyConfig::kStart) || KeyConfig::Down(KeyConfig::kBack))
	{
		m_bgmStream.setFadeOut(0.5); // TODO: フェードアウト時間の定数を統一
		requestNextScene<SelectScene>();
		return;
	}
}

void ResultScene::draw() const
{
	FitToHeight(m_bgTexture).drawAt(Scene::Center());

	m_resultPanel.draw();
}

Co::Task<void> ResultScene::fadeIn()
{
	co_await Co::SimpleFadeIn(kFadeDuration, Palette::White);
}
