#pragma once

namespace MusicGame::Judgment
{
	enum class JudgmentResult
	{
		kUnspecified,
		kError,
		kNear,
		kCritical,
	};

	enum class KeyBeamType : int32
	{
		kDefault = 0,
		kNear,
		kCritical,
		kNearSlow,
		kNearFast,
		kErrorFast,

		kNumBeamTypes,
	};

	namespace TimingWindow
	{
		namespace ChipNote
		{
			constexpr double kWindowSecCritical = 0.043;
			constexpr double kWindowSecNear = 0.113;
			constexpr double kWindowSecError = 0.260;
			constexpr double kWindowSecErrorEasy = 0.160;

			constexpr JudgmentResult Judge(double secDiff, bool isEasyGauge)
			{
				if (secDiff < kWindowSecCritical)
				{
					return JudgmentResult::kCritical;
				}
				else if (secDiff < kWindowSecNear)
				{
					return JudgmentResult::kNear;
				}
				else if (secDiff < (isEasyGauge ? kWindowSecErrorEasy : kWindowSecError))
				{
					return JudgmentResult::kError;
				}
				else
				{
					return JudgmentResult::kUnspecified;
				}
			}
		}

		namespace LongNote
		{
			constexpr double kWindowSecPreHold = 0.15;
		}
	}

	constexpr int32 kScoreValueNear = 1;
	constexpr int32 kScoreValueCritical = 2;
}
