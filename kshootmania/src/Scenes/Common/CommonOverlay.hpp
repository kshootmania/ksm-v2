#pragma once

/// @brief 全画面共通のオーバーレイ表示(Backボタン)
class CommonOverlay
{
private:
	std::shared_ptr<noco::Canvas> m_canvas;

	/// @brief Backボタンを長押しで反応させる場合の必要時間(noneの場合はクリックで即時反応)
	Optional<Duration> m_backButtonHoldDuration = none;

	bool m_backButtonPressed = false;

	/// @brief Backボタンの押下継続時間(秒)
	double m_backButtonHoldSec = 0.0;

	/// @brief Backボタンのゲージ進捗(0〜1)
	double m_backButtonProgress = 0.0;

public:
	CommonOverlay();

	explicit CommonOverlay(const Duration& backButtonHoldDuration);

	void update(noco::HitTestEnabledYN hitTestEnabled);

	void draw() const;

	void setBackButtonVisible(bool visible);

	/// @brief Backボタンの長押し必要時間を切り替える(noneの場合は長押しなし)
	void setBackButtonHoldDuration(const Optional<Duration>& holdDuration);
};
