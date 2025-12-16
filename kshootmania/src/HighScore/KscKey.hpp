#pragma once
#include "MusicGame/GameDefines.hpp"

struct KscKey
{
	GaugeType gaugeType = GaugeType::kNormalGauge;

	TurnMode turnMode = TurnMode::kNormal;

	double playbackSpeed = 1.0;

	JudgmentPlayMode btPlayMode = JudgmentPlayMode::kOn;

	JudgmentPlayMode fxPlayMode = JudgmentPlayMode::kOn;

	JudgmentPlayMode laserPlayMode = JudgmentPlayMode::kOn;

	[[nodiscard]]
	String toString() const;

	KscKey withGaugeType(GaugeType newGaugeType) const;
};
