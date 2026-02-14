#pragma once
#include "HighScore/HighScoreInfo.hpp"
#include "Input/KeyConfig.hpp"
#include <Siv3D.hpp>
#include <NocoUI.hpp>

// BTボタン3つ以上同時押し中に表示されるプレイ統計パネル
class PlayStatsPanel
{
private:
	std::shared_ptr<noco::Canvas> m_canvas;

	// 現在表示されているか
	bool m_isVisible = false;

	// BTボタン3つ以上が押されているかチェック
	[[nodiscard]]
	bool isThreeBTButtonsPressed() const;

	// 統計情報テキストを生成
	[[nodiscard]]
	String generateStatsText(const HighScoreInfo& highScore, GaugeType gaugeType) const;

public:
	explicit PlayStatsPanel(std::shared_ptr<noco::Canvas> canvas);

	// 更新
	void update(const Optional<HighScoreInfo>& highScore, GaugeType gaugeType);

	// パネルが表示されているかどうか
	[[nodiscard]]
	bool isVisible() const;
};
