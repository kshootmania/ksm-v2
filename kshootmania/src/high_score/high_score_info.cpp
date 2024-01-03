#include "high_score_info.hpp"

int32 HighScoreInfo::score(GaugeType gaugeType) const
{
	// NORMALとHARDは同じスコア基準なので共有、EASYはタイミング判定しきい値が異なりスコア基準が異なるので別にする
	switch (gaugeType)
	{
	case GaugeType::kEasyGauge:
		return easyGauge.score;

	case GaugeType::kNormalGauge:
	case GaugeType::kHardGauge:
		return Max(normalGauge.score, hardGauge.score);

	default:
		assert(false && "Unknown gauge type");
		return 0;
	}
}

Medal HighScoreInfo::medal() const
{
	// NORMALとHARDのPERFECT/FULLCOMBOは同じなので共有、EASYはタイミング判定しきい値が異なるので別にする

	if (normalGauge.achievement == Achievement::kPerfect || hardGauge.achievement == Achievement::kPerfect)
	{
		return Medal::kPerfect;
	}

	if (normalGauge.achievement == Achievement::kFullCombo || hardGauge.achievement == Achievement::kFullCombo)
	{
		return Medal::kFullCombo;
	}

	if (hardGauge.achievement == Achievement::kCleared)
	{
		return Medal::kHardClear;
	}

	if (normalGauge.achievement == Achievement::kCleared)
	{
		return Medal::kClear;
	}

	if (easyGauge.achievement == Achievement::kPerfect)
	{
		return Medal::kEasyPerfect;
	}

	if (easyGauge.achievement == Achievement::kFullCombo)
	{
		return Medal::kEasyFullCombo;
	}

	if (easyGauge.achievement == Achievement::kCleared)
	{
		return Medal::kEasyClear;
	}

	return Medal::kNoMedal;
}

Grade HighScoreInfo::grade(GaugeType gaugeType) const
{
	// NORMALとHARDは同じグレード基準なので共有、EASYはタイミング判定しきい値が異なりグレード基準が異なるので別にする
	// (NORMALとHARDではパーセンテージ基準は異なるが、HARDのグレード算出の際はNORMAL扱いでのパーセンテージを使用するためグレード基準は同じ)
	switch (gaugeType)
	{
	case GaugeType::kEasyGauge:
		return easyGauge.grade;

	case GaugeType::kNormalGauge:
	case GaugeType::kHardGauge:
		return Max(normalGauge.grade, hardGauge.grade);

	default:
		assert(false && "Unknown gauge type");
		return Grade::kNoGrade;
	}
}

int32 HighScoreInfo::percent(GaugeType gaugeType) const
{
	// ゲージのパーセンテージについてはEASY、NORMAL、HARDで全て異なるので別にする
	switch (gaugeType)
	{
	case GaugeType::kEasyGauge:
		return easyGauge.percent;

	case GaugeType::kNormalGauge:
		return normalGauge.percent;

	case GaugeType::kHardGauge:
		return hardGauge.percent;

	default:
		assert(false && "Unknown gauge type");
		return 0;
	}
}

int32 HighScoreInfo::playCount(GaugeType gaugeType) const
{
	// 主にCLEAR/FULLCOMBO/PERFECTの分母として見る目的の数値なので、それらと同様にNORMALとHARDは共有、EASYは別にする
	switch (gaugeType)
	{
	case GaugeType::kEasyGauge:
		return easyGauge.playCount;

	case GaugeType::kNormalGauge:
	case GaugeType::kHardGauge:
		return normalGauge.playCount + hardGauge.playCount;

	default:
		assert(false && "Unknown gauge type");
		return 0;
	}
}

int32 HighScoreInfo::clearCount(GaugeType gaugeType) const
{
	// NORMALとHARDのCLEARは同じなので共有、EASYはタイミング判定しきい値が異なりスコア基準が異なるので別にする
	switch (gaugeType)
	{
	case GaugeType::kEasyGauge:
		return easyGauge.clearCount;

	case GaugeType::kNormalGauge:
	case GaugeType::kHardGauge:
		return normalGauge.clearCount + hardGauge.clearCount;

	default:
		assert(false && "Unknown gauge type");
		return 0;
	}
}

int32 HighScoreInfo::fullComboCount(GaugeType gaugeType) const
{
	// NORMALとHARDは同じスコア基準なので共有、EASYはタイミング判定しきい値が異なりスコア基準が異なるので別にする
	switch (gaugeType)
	{
	case GaugeType::kEasyGauge:
		return easyGauge.fullComboCount;

	case GaugeType::kNormalGauge:
	case GaugeType::kHardGauge:
		return normalGauge.fullComboCount + hardGauge.fullComboCount;

	default:
		assert(false && "Unknown gauge type");
		return 0;
	}
}

int32 HighScoreInfo::perfectCount(GaugeType gaugeType) const
{
	// NORMALとHARDは同じスコア基準なので共有、EASYはタイミング判定しきい値が異なりスコア基準が異なるので別にする
	switch (gaugeType)
	{
	case GaugeType::kEasyGauge:
		return easyGauge.perfectCount;

	case GaugeType::kNormalGauge:
	case GaugeType::kHardGauge:
		return normalGauge.perfectCount + hardGauge.perfectCount;

	default:
		assert(false && "Unknown gauge type");
		return 0;
	}
}

int32 HighScoreInfo::maxCombo(GaugeType gaugeType) const
{
	// NORMALとHARDは同じスコア基準なので共有、EASYはタイミング判定しきい値が異なりスコア基準が異なるので別にする
	switch (gaugeType)
	{
	case GaugeType::kEasyGauge:
		return easyGauge.maxCombo;

	case GaugeType::kNormalGauge:
	case GaugeType::kHardGauge:
		return Max(normalGauge.maxCombo, hardGauge.maxCombo);

	default:
		assert(false && "Unknown gauge type");
		return 0;
	}
}

KscValue& HighScoreInfo::kscValueOf(GaugeType gaugeType)
{
	switch (gaugeType)
	{
	case GaugeType::kEasyGauge:
		return easyGauge;

	case GaugeType::kNormalGauge:
		return normalGauge;

	case GaugeType::kHardGauge:
		return hardGauge;

	default:
		throw Error(U"Unknown gauge type");
	}
}

const KscValue& HighScoreInfo::kscValueOf(GaugeType gaugeType) const
{
	switch (gaugeType)
	{
	case GaugeType::kEasyGauge:
		return easyGauge;

	case GaugeType::kNormalGauge:
		return normalGauge;

	case GaugeType::kHardGauge:
		return hardGauge;

	default:
		throw Error(U"Unknown gauge type");
	}
}

bool HighScoreInfo::isHighScoreUpdated(const MusicGame::PlayResult& playResult) const
{
	// NORMALとHARDで共通のハイスコア上で更新判定する必要があるため、
	// ここではkscValueOf関数ではなくscore関数を使用して現在のスコアを調べる必要がある点に注意
	return score(playResult.playOption.gaugeType) < playResult.score;
}

HighScoreInfo HighScoreInfo::applyPlayResult(const MusicGame::PlayResult& playResult) const
{
	HighScoreInfo newHighScoreInfo = *this;
	newHighScoreInfo.kscValueOf(playResult.playOption.gaugeType) = kscValueOf(playResult.playOption.gaugeType).applyPlayResult(playResult);
	return newHighScoreInfo;
}
