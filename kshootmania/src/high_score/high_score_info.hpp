#pragma once
#include "ksc_value.hpp"

struct HighScoreInfo
{
	KscValue normalGauge;

	KscValue easyGauge;

	KscValue hardGauge;

	int32 score(GaugeType gaugeType) const;

	Medal medal() const;

	Grade grade(GaugeType gaugeType) const;

	int32 percent(GaugeType gaugeType) const;

	int32 playCount(GaugeType gaugeType) const;

	int32 clearCount(GaugeType gaugeType) const;

	int32 fullComboCount(GaugeType gaugeType) const;

	int32 perfectCount(GaugeType gaugeType) const;

	int32 maxCombo(GaugeType gaugeType) const;
};
