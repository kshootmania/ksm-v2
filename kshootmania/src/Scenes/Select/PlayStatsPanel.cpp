#include "PlayStatsPanel.hpp"
#include "I18n/I18n.hpp"

PlayStatsPanel::PlayStatsPanel(std::shared_ptr<noco::Canvas> canvas)
	: m_canvas(canvas)
{
}

bool PlayStatsPanel::isThreeBTButtonsPressed() const
{
	int32 btPressedCount = 0;
	for (Button btButton = kButtonBT_A; btButton <= kButtonBT_D; ++btButton)
	{
		if (KeyConfig::Pressed(btButton))
		{
			++btPressedCount;
		}
	}
	return btPressedCount >= 3;
}

String PlayStatsPanel::generateStatsText(const HighScoreInfo& highScore, GaugeType gaugeType) const
{
	const int32 playCount = highScore.playCount(gaugeType);
	const int32 clearCount = highScore.clearCount(gaugeType);
	const int32 fullComboCount = highScore.fullComboCount(gaugeType);
	const int32 perfectCount = highScore.perfectCount(gaugeType);

	return U"{}{}\n{}{}\n{}{}\n{}{}"_fmt(
		I18n::Get(I18n::Select::PlayStatsPlayCount),
		playCount,
		I18n::Get(I18n::Select::PlayStatsClear),
		clearCount,
		I18n::Get(I18n::Select::PlayStatsFullCombo),
		fullComboCount,
		I18n::Get(I18n::Select::PlayStatsPerfect),
		perfectCount
	);
}

void PlayStatsPanel::update(const Optional<HighScoreInfo>& highScore, GaugeType gaugeType)
{
	const bool shouldBeVisible = isThreeBTButtonsPressed() && highScore.has_value();

	if (shouldBeVisible)
	{
		// 統計情報テキストを生成してCanvasに設定
		const String statsText = generateStatsText(*highScore, gaugeType);
		m_canvas->setParamValue(U"overlay_playStatsPanelText", statsText);
	}

	// パネルの表示状態を更新
	m_canvas->setParamValue(U"overlay_playStatsPanelVisible", shouldBeVisible);
	m_isVisible = shouldBeVisible;
}

bool PlayStatsPanel::isVisible() const
{
	return m_isVisible;
}

void PlayStatsPanel::hide()
{
	m_isVisible = false;
	m_canvas->setParamValue(U"overlay_playStatsPanelVisible", false);
}
