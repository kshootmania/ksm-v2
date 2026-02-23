#pragma once

namespace MusicGame
{
	enum class GameMode
	{
		kNormal,
		kCourseMode,
	};

	enum class GaugeCalcType
	{
		kNormalEasy,
		kNormalNormal,
		kNormalHard,
		kCourseEasy,
		kCourseNormal,
		kCourseHard,
	};

	// GaugeTypeとGameModeからGaugeCalcTypeを取得
	inline GaugeCalcType ToGaugeCalcType(GaugeType gaugeType, GameMode gameMode)
	{
		if (gameMode == GameMode::kCourseMode)
		{
			if (gaugeType == GaugeType::kEasyGauge)
			{
				return GaugeCalcType::kCourseEasy;
			}
			else if (gaugeType == GaugeType::kNormalGauge)
			{
				return GaugeCalcType::kCourseNormal;
			}
			else
			{
				return GaugeCalcType::kCourseHard;
			}
		}
		else
		{
			if (gaugeType == GaugeType::kEasyGauge)
			{
				return GaugeCalcType::kNormalEasy;
			}
			else if (gaugeType == GaugeType::kNormalGauge)
			{
				return GaugeCalcType::kNormalNormal;
			}
			else
			{
				return GaugeCalcType::kNormalHard;
			}
		}
	}

	// GaugeCalcTypeからGaugeTypeを取得
	inline GaugeType ToGaugeType(GaugeCalcType gaugeCalcType)
	{
		switch (gaugeCalcType)
		{
		case GaugeCalcType::kNormalEasy:
		case GaugeCalcType::kCourseEasy:
			return GaugeType::kEasyGauge;
		case GaugeCalcType::kNormalNormal:
		case GaugeCalcType::kCourseNormal:
			return GaugeType::kNormalGauge;
		case GaugeCalcType::kNormalHard:
		case GaugeCalcType::kCourseHard:
			return GaugeType::kHardGauge;
		default:
			return GaugeType::kNormalGauge;
		}
	}

	// GaugeCalcTypeがコースモードかどうか
	inline bool IsCourseGaugeCalcType(GaugeCalcType gaugeCalcType)
	{
		return gaugeCalcType == GaugeCalcType::kCourseEasy ||
			gaugeCalcType == GaugeCalcType::kCourseNormal ||
			gaugeCalcType == GaugeCalcType::kCourseHard;
	}

	constexpr Duration kTimeSecBeforeStart = 3.4s;
	constexpr Duration kTimeSecBeforeStartMovie = 4.4s;

	constexpr Duration TimeSecBeforeStart(bool movie)
	{
		return movie ? kTimeSecBeforeStartMovie : kTimeSecBeforeStart;
	}

	constexpr kson::Pulse kPastPulse = -100000000;

	constexpr int32 kScoreMax = 10000000;

	constexpr int32 kGaugeValueChip = 200;
	constexpr int32 kGaugeValueChipNear = 50;
	constexpr int32 kGaugeValueLong = 50;

	constexpr int32 kGaugeValueMaxHard = 100000;

	constexpr int32 kGaugePercentageThreshold = 70;
	constexpr int32 kGaugePercentageThresholdHard = 0;
	constexpr int32 kGaugePercentageThresholdHardWarning = 30;

	constexpr double kGaugeDecreasePercentByChipError = 2.0;
	constexpr double kGaugeDecreasePercentByLongError = 0.5;

	// HARDゲージの固定減少値
	constexpr int32 kGaugeDecreaseValueHardChipError = 9000; // チップ/直角LASERエラー時
	constexpr int32 kGaugeDecreaseValueHardLongError = 2250; // ロング/LASERエラー時

	// コースモードのゲージ減少基本値
	constexpr int32 kGaugeDecreaseBaseValueCourseChipError = 4; // チップ/直角LASERエラー時
	constexpr int32 kGaugeDecreaseBaseValueCourseLongError = 1; // ロング/LASERエラー時

	// ゲージ種類別の上昇率倍率
	constexpr double kGaugeIncreaseRateEasy = 1.10; // EASY
	constexpr double kGaugeIncreaseRateNormal = 1.00; // NORMAL
	constexpr double kGaugeIncreaseRateHard = 0.60; // HARD

	// ゲージ種類別の下降率倍率(EASY/NORMAL用)
	constexpr double kGaugeDecreaseRateEasy = 0.75; // EASY: 25%減
	constexpr double kGaugeDecreaseRateNormal = 1.00; // NORMAL: 基準

	// HARDゲージが一定以下の場合の減少率軽減倍率
	constexpr double kGaugeDecreaseRateHardLow = 0.59; // 41%軽減

	// コースモード用のゲージ増加率倍率
	constexpr double kGaugeIncreaseRateCourseEasy = 1.10; // EASY
	constexpr double kGaugeIncreaseRateCourseNormal = 0.5; // NORMAL
	constexpr double kGaugeIncreaseRateCourseHard = 0.4; // HARD

	// コースモード用のゲージ減少率倍率
	constexpr double kGaugeDecreaseRateCourseNormal = 850.0;
	constexpr double kGaugeDecreaseRateCourseEasy = 637.5;
	constexpr double kGaugeDecreaseRateCourseHard = 3037.5;

	// コースモードでゲージが一定以下の場合の減少率軽減倍率
	constexpr double kGaugeDecreaseRateCourseLow = 10.0 / 17.0; // 約41%軽減
	constexpr double kGaugeDecreaseRateCourseHardLow = 0.59; // 41%軽減

	constexpr double kTiltRadians = 10_deg;
	constexpr double kBgLayerTiltMaxRadians = 14_deg;

	using IsAutoPlayYN = YesNo<struct IsAutoPlayYN_tag>;
	using IsHardFailedYN = YesNo<struct IsHardFailedYN_tag>;

	// Turn変換テーブル
	struct TurnTable
	{
		std::array<std::size_t, kson::kNumBTLanesSZ> btLaneTable; // BTレーン変換テーブル

		bool invertFXLane; // FXレーン左右反転

		bool invertLaserLane; // LASERレーン左右反転

		bool invertLaserValue; // LASER値左右反転(カメラエフェクト左右反転にも使用)
	};

	// TurnMode から TurnTable を生成
	inline TurnTable MakeTurnTable(TurnMode turnMode)
	{
		if (turnMode == TurnMode::kNormal)
		{
			return TurnTable{
				.btLaneTable = { 0, 1, 2, 3 },
				.invertFXLane = false,
				.invertLaserLane = false,
				.invertLaserValue = false,
			};
		}
		else if (turnMode == TurnMode::kMirror)
		{
			return TurnTable{
				.btLaneTable = { 3, 2, 1, 0 }, // D,C,B,A
				.invertFXLane = true, // R,L
				.invertLaserLane = true, // R,L
				.invertLaserValue = true, // 1.0 - 値
			};
		}
		else // TurnMode::kRandom
		{
			// BTレーンのランダムテーブル生成
			std::array<std::size_t, kson::kNumBTLanesSZ> btLaneTable = { 0, 1, 2, 3 };
			std::shuffle(btLaneTable.begin(), btLaneTable.end(), GetDefaultRNG());

			// FX/LASERレーンのランダム左右反転
			const bool invertFXLane = Random(0, 1) == 1;
			const bool invertLaserLane = Random(0, 1) == 1;

			return TurnTable{
				.btLaneTable = btLaneTable,
				.invertFXLane = invertFXLane,
				.invertLaserLane = invertLaserLane,
				.invertLaserValue = invertLaserLane, // レーザー反転と同じ
			};
		}
	}
}
