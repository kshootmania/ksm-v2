#include "KscKey.hpp"

namespace
{
	[[nodiscard]]
	StringView GaugeTypeStr(GaugeType gaugeType) noexcept
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

	[[nodiscard]]
	StringView TurnModeStr(TurnMode turnMode) noexcept
	{
		switch (turnMode)
		{
		case TurnMode::kNormal:
			return U"normal";
		case TurnMode::kMirror:
			return U"mirror";
		case TurnMode::kRandom:
			return U"random";
		default:
			assert(false && "Unknown turn mode");
			return U"?";
		}
	}

	[[nodiscard]]
	StringView JudgmentPlayModeStr(JudgmentPlayMode judgmentPlayMode) noexcept
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

	[[nodiscard]]
	String PlaybackSpeedStr(double playbackSpeed)
	{
		if (playbackSpeed == 1.0)
		{
			return U"normal";
		}
		const int32 speedInt = static_cast<int32>(Round(playbackSpeed * 10.0));
		return U"x{:02}"_fmt(speedInt);
	}
}

[[nodiscard]]
String KscKey::toString() const
{
	return U"{},{},{},{},{},{}"_fmt(
		GaugeTypeStr(gaugeType),
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
