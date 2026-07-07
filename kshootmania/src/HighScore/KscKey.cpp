#include "KscKey.hpp"

namespace
{
	StringView GaugeTypeStr(GaugeType gaugeType)
	{
		switch (gaugeType)
		{
		case GaugeType::kEasyGauge:
			return U"easy";
		case GaugeType::kNormalGauge:
			return U"normal";
		case GaugeType::kHardGauge:
			return U"hard";
		default:
			assert(false && "Unknown gauge type");
			return U"?";
		}
	}

	StringView TurnModeStr(TurnMode turnMode)
	{
		switch (turnMode)
		{
		case TurnMode::kNormal:
			return U"normal";
		case TurnMode::kMirror:
			return U"mirror";
		case TurnMode::kRandom:
			return U"random";
		case TurnMode::kSRandom:
			return U"s-random";
		default:
			assert(false && "Unknown turn mode");
			return U"?";
		}
	}

	StringView JudgmentPlayModeStr(JudgmentPlayMode judgmentPlayMode)
	{
		switch (judgmentPlayMode)
		{
		case JudgmentPlayMode::kOn:
			return U"on";
		case JudgmentPlayMode::kOff:
			return U"off";
		case JudgmentPlayMode::kAuto:
			return U"auto";
		case JudgmentPlayMode::kHide:
			return U"hide";
		default:
			assert(false && "Unknown judgment play mode");
			return U"?";
		}
	}

	String PlaybackSpeedStr(double playbackSpeed)
	{
		if (playbackSpeed == 1.0)
		{
			return U"normal";
		}

		const int32 speedPercent = static_cast<int32>(Round(playbackSpeed * 100.0));
		// 10%刻みは「x09」など2桁(v2.0.0-alpha5との互換性のため)
		if (speedPercent % 10 == 0)
		{
			return U"x{:02}"_fmt(speedPercent / 10);
		}
		// それ以外は「x095」など3桁
		return U"x{:03}"_fmt(speedPercent);
	}
}

String KscKey::toString() const
{
	return U"{},{}"_fmt(GaugeTypeStr(gaugeType), toStringWithoutGaugeType());
}

String KscKey::toStringWithoutGaugeType() const
{
	return U"{},{},{},{},{}"_fmt(
		TurnModeStr(turnMode),
		PlaybackSpeedStr(playbackSpeed),
		JudgmentPlayModeStr(btPlayMode),
		JudgmentPlayModeStr(fxPlayMode),
		JudgmentPlayModeStr(laserPlayMode));
}

KscKey KscKey::withGaugeType(GaugeType newGaugeType) const
{
	KscKey result = *this;
	result.gaugeType = newGaugeType;
	return result;
}
