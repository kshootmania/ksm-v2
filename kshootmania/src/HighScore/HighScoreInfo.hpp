#pragma once
#include "KscValue.hpp"
#include "MusicGame/PlayResult.hpp"

struct HighScoreInfo
{
	KscValue normalGauge;

	KscValue easyGauge;

	KscValue hardGauge;

	[[nodiscard]]
	int32 score(GaugeType gaugeType) const noexcept;

	[[nodiscard]]
	Medal medal() const noexcept;

	[[nodiscard]]
	Grade grade(GaugeType gaugeType) const noexcept;

	[[nodiscard]]
	int32 percent(GaugeType gaugeType) const noexcept;

	[[nodiscard]]
	int32 playCount(GaugeType gaugeType) const noexcept;

	[[nodiscard]]
	int32 clearCount(GaugeType gaugeType) const noexcept;

	[[nodiscard]]
	int32 fullComboCount(GaugeType gaugeType) const noexcept;

	[[nodiscard]]
	int32 perfectCount(GaugeType gaugeType) const noexcept;

	[[nodiscard]]
	int32 maxCombo(GaugeType gaugeType) const noexcept;

	/// @brief ゲージの種類に対応するKscValueへの参照
	/// @param gaugeType ゲージの種類
	/// @return KscValue
	/// @throws Error
	/// @remark この関数で得られるKscValueは各ゲージ毎での生の情報なので、ゲージ間で共有されるべき
	///		    ハイスコア情報(NORMALとHARDのハイスコアが共通になっている、など)は加味されていない点に注意。
	/// 		そのため、それらを加味したハイスコア情報が必要な時はscore関数等を使用すること。
	[[nodiscard]]
	KscValue& kscValueOf(GaugeType gaugeType);

	/// @brief ゲージの種類に対応するKscValueへの参照
	/// @param gaugeType ゲージの種類
	/// @return KscValue
	/// @throws Error
	/// @remark この関数で得られるKscValueは各ゲージ毎での生の情報なので、ゲージ間で共有されるべき
	///		    ハイスコア情報(NORMALとHARDのハイスコアが共通になっている、など)は加味されていない点に注意。
	/// 		そのため、それらを加味したハイスコア情報が必要な時はscore関数等を使用すること。
	[[nodiscard]]
	const KscValue& kscValueOf(GaugeType gaugeType) const;

	/// @brief ハイスコアを上回ったかを返す
	/// @param playResult プレイ結果
	/// @return ハイスコアを上回った場合はtrue, そうでなければfalse
	[[nodiscard]]
	bool isHighScoreUpdated(const MusicGame::PlayResult& playResult) const;

	/// @brief プレイ結果を反映後のハイスコア情報を返す
	/// @param playResult プレイ結果
	[[nodiscard]]
	HighScoreInfo applyPlayResult(const MusicGame::PlayResult& playResult) const;
};
