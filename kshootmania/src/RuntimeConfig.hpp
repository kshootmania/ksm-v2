#pragma once
#include "Common/CommonDefines.hpp"

// 実行中のみ保持する設定
// config.iniには保存されず、起動時にデフォルト値にリセットされる
namespace RuntimeConfig
{
	[[nodiscard]]
	GaugeType GetGaugeType() noexcept;

	void SetGaugeType(GaugeType gaugeType) noexcept;

	[[nodiscard]]
	TurnMode GetTurnMode() noexcept;

	void SetTurnMode(TurnMode turnMode) noexcept;

	[[nodiscard]]
	JudgmentPlayMode GetJudgmentPlayModeBT() noexcept;

	void SetJudgmentPlayModeBT(JudgmentPlayMode mode) noexcept;

	[[nodiscard]]
	JudgmentPlayMode GetJudgmentPlayModeFX() noexcept;

	void SetJudgmentPlayModeFX(JudgmentPlayMode mode) noexcept;

	[[nodiscard]]
	JudgmentPlayMode GetJudgmentPlayModeLaser() noexcept;

	void SetJudgmentPlayModeLaser(JudgmentPlayMode mode) noexcept;

	[[nodiscard]]
	double GetPlaybackSpeed() noexcept;

	void SetPlaybackSpeed(double speed) noexcept;

	// config.iniから判定モードを復元
	void RestoreJudgmentModesFromConfigIni();
}
