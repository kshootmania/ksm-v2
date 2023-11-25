#include "ksc_key.hpp"

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
}

String KscKey::toString() const
{
	// 3番目は元々ノーツ増減オプション用に予約されていたが不使用なので"normal"固定
	return U"{},{},normal,{},{},{}"_fmt(
		GaugeTypeStr(gaugeType),
		TurnModeStr(turnMode),
		JudgmentPlayModeStr(btPlayMode),
		JudgmentPlayModeStr(fxPlayMode),
		JudgmentPlayModeStr(laserPlayMode));
}

KscKey KscKey::withGaugeType(GaugeType gaugeType) const
{
	KscKey result = *this;
	result.gaugeType = gaugeType;
	return result;
}
