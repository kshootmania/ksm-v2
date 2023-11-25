#pragma once
#include "music_game/game_defines.hpp"

struct KscKey
{
	GaugeType gaugeType = GaugeType::kNormalGauge;

	TurnMode turnMode = TurnMode::kNormal;

	JudgmentPlayMode btPlayMode = JudgmentPlayMode::kOn;

	JudgmentPlayMode fxPlayMode = JudgmentPlayMode::kOn;

	JudgmentPlayMode laserPlayMode = JudgmentPlayMode::kOn;

	String toString() const;

	KscKey withGaugeType(GaugeType gaugeType) const;
};
