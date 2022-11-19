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

		namespace LaserNote
		{
			constexpr double kWindowSecSlam = 0.06;
		}
	}

	constexpr int32 kScoreValueNear = 1;
	constexpr int32 kScoreValueCritical = 2;

	constexpr double kHalveComboBPMThreshold = 256.0;

	// キーボード入力での1秒あたりのLASERカーソル移動量
	constexpr double kLaserKeyboardCursorXPerSec = 3.0;

	// LASERカーソル位置をCRITICAL判定とする許容誤差
	constexpr double kLaserCriticalMaxDeltaCursorX = 0.035;

	// LASERカーソルの増幅移動量の倍率
	// (LASERカーソル移動がLASERノーツと同方向の場合はこの倍率で入力があったものと見立てた"増幅移動量"を計算し、
	//  "増幅移動量"での移動幅が理想位置を超えている場合は理想位置にカーソルを吸い付かせる)
	constexpr double kLaserCursorInputOvershootScale = 7.5;

	// 直角LASERをCRITICAL判定するのに必要な累計カーソル移動量
	constexpr double kLaserSlamCriticalDeltaCursorXThreshold = 0.035;

	// 直角LASER判定後の判定補正時間
	constexpr double kLaserAutoSecAfterSlamJudgment = 0.075;
}
