#include "ScoringStatus.hpp"

namespace MusicGame::Judgment
{

	void ScoringStatus::addGaugeValue(int32 add)
	{
		int32 adjustedAdd = 0;

		switch (m_gaugeCalcType)
		{
		case GaugeCalcType::kNormalEasy:
			adjustedAdd = static_cast<int32>(add * kGaugeIncreaseRateEasy);
			m_gaugeValue = Min(m_gaugeValue + adjustedAdd, m_gaugeValueMax);
			break;

		case GaugeCalcType::kNormalNormal:
			adjustedAdd = static_cast<int32>(add * kGaugeIncreaseRateNormal);
			m_gaugeValue = Min(m_gaugeValue + adjustedAdd, m_gaugeValueMax);
			break;

		case GaugeCalcType::kNormalHard:
			adjustedAdd = static_cast<int32>(add * kGaugeIncreaseRateHard);
			m_gaugeValue = Min(m_gaugeValue + adjustedAdd, kGaugeValueMaxHard);
			break;

		case GaugeCalcType::kCourseNormal:
			adjustedAdd = static_cast<int32>(kGaugeValueMaxHard * add * kGaugeIncreaseRateCourseNormal / m_gaugeValueMax);
			m_gaugeValue = Min(m_gaugeValue + adjustedAdd, kGaugeValueMaxHard);
			break;

		case GaugeCalcType::kCourseEasy:
			adjustedAdd = static_cast<int32>(kGaugeValueMaxHard * add * kGaugeIncreaseRateCourseEasy / m_gaugeValueMax);
			m_gaugeValue = Min(m_gaugeValue + adjustedAdd, kGaugeValueMaxHard);
			break;

		case GaugeCalcType::kCourseHard:
			adjustedAdd = static_cast<int32>(add * kGaugeIncreaseRateCourseHard);
			m_gaugeValue = Min(m_gaugeValue + adjustedAdd, kGaugeValueMaxHard);
			break;
		}

		// Grade計算用にNORMAL基準のゲージ値も更新
		addGaugeValueNormal(add);
	}

	void ScoringStatus::addGaugeValueNormal(int32 add)
	{
		const int32 normalAdd = static_cast<int32>(add * kGaugeIncreaseRateNormal);
		m_gaugeValueNormal = Min(m_gaugeValueNormal + normalAdd, m_gaugeValueMax);
	}

	void ScoringStatus::subtractGaugeValue(int32 sub)
	{
		int32 adjustedSub = 0;
		const int32 currentPercentageInt = gaugePercentageInt(m_gaugeType);

		switch (m_gaugeCalcType)
		{
		case GaugeCalcType::kNormalEasy:
			adjustedSub = static_cast<int32>(sub * kGaugeDecreaseRateEasy);
			break;

		case GaugeCalcType::kNormalNormal:
			adjustedSub = static_cast<int32>(sub * kGaugeDecreaseRateNormal);
			break;

		case GaugeCalcType::kNormalHard:
			{
				const double rate = currentPercentageInt <= kGaugePercentageThresholdHardWarning ? kGaugeDecreaseRateHardLow : 1.0;
				adjustedSub = static_cast<int32>(sub * rate);
				break;
			}

		case GaugeCalcType::kCourseNormal:
			{
				const double rate = currentPercentageInt <= kGaugePercentageThresholdHardWarning ? kGaugeDecreaseRateCourseLow : 1.0;
				adjustedSub = static_cast<int32>(sub * kGaugeDecreaseRateCourseNormal * rate);
				break;
			}

		case GaugeCalcType::kCourseEasy:
			{
				const double rate = currentPercentageInt <= kGaugePercentageThresholdHardWarning ? kGaugeDecreaseRateCourseLow : 1.0;
				adjustedSub = static_cast<int32>(sub * kGaugeDecreaseRateCourseEasy * rate);
				break;
			}

		case GaugeCalcType::kCourseHard:
			{
				const double rate = currentPercentageInt <= kGaugePercentageThresholdHardWarning ? kGaugeDecreaseRateCourseHardLow : 1.0;
				adjustedSub = static_cast<int32>(sub * kGaugeDecreaseRateCourseHard * rate);
				break;
			}
		}

		m_gaugeValue = Max(m_gaugeValue - adjustedSub, 0);

		// Grade計算用にNORMAL基準のゲージ値も更新
		const int32 normalSub = static_cast<int32>(sub * kGaugeDecreaseRateNormal);
		m_gaugeValueNormal = Max(m_gaugeValueNormal - normalSub, 0);
	}

	ScoringStatus::ScoringStatus(int32 scoreValueMax, int32 gaugeValueMax, GaugeType gaugeType, const Optional<CourseContinuation>& courseContinuation, GameMode gameMode)
		: m_scoreValueMax(scoreValueMax)
		, m_gaugeValueMax(gaugeValueMax)
		, m_gaugeType(gaugeType)
		, m_gameMode(gameMode)
		, m_gaugeCalcType(ToGaugeCalcType(gaugeType, gameMode))
		, m_gaugeValue(courseContinuation.has_value() ? courseContinuation->gaugeValue : ((gaugeType == GaugeType::kHardGauge || gameMode == GameMode::kCourseMode) ? kGaugeValueMaxHard : 0))
		, m_comboStatus(courseContinuation)
	{
	}

	void ScoringStatus::onChipOrLaserSlamJudgment(Judgment::JudgmentResult result)
	{
		m_comboStatus.processJudgmentResult(result);

		switch (result)
		{
		case Judgment::JudgmentResult::kCritical:
			m_scoreValue += Judgment::kScoreValueCritical;
			addGaugeValue(kGaugeValueChip);
			break;

		case Judgment::JudgmentResult::kNearFast:
		case Judgment::JudgmentResult::kNearSlow:
			m_scoreValue += Judgment::kScoreValueNear;
			if (m_gaugeCalcType == GaugeCalcType::kCourseHard)
			{
				// コースモードのHARDゲージでは、NEARでゲージが減る
				subtractGaugeValue(kGaugeDecreaseBaseValueCourseChipError);
			}
			else if (m_gaugeType != GaugeType::kHardGauge)
			{
				// 通常モードのEASY/NORMALゲージでは、NEARでゲージが増える
				addGaugeValue(kGaugeValueChipNear);
			}
			else
			{
				// 通常モードのHARDゲージでは、実ゲージは増減なしだが、グレード計算用のNORMAL基準ゲージは増やす
				addGaugeValueNormal(kGaugeValueChipNear);
			}
			break;

		case Judgment::JudgmentResult::kError:
			if (m_gaugeCalcType == GaugeCalcType::kCourseEasy || m_gaugeCalcType == GaugeCalcType::kCourseNormal || m_gaugeCalcType == GaugeCalcType::kCourseHard)
			{
				subtractGaugeValue(kGaugeDecreaseBaseValueCourseChipError);
			}
			else if (m_gaugeCalcType == GaugeCalcType::kNormalHard)
			{
				subtractGaugeValue(kGaugeDecreaseValueHardChipError);
			}
			else
			{
				subtractGaugeValue(static_cast<int32>(m_gaugeValueMax * kGaugeDecreasePercentByChipError / 100));
			}
			break;

		default:
			assert(false && "Invalid JudgmentResult in doChipJudgment");
			break;
		}
	}

	void ScoringStatus::onLongOrLaserLineJudgment(Judgment::JudgmentResult result)
	{
		m_comboStatus.processJudgmentResult(result);

		switch (result)
		{
		case Judgment::JudgmentResult::kCritical:
			m_scoreValue += Judgment::kScoreValueCritical;
			addGaugeValue(kGaugeValueLong);
			break;

		case Judgment::JudgmentResult::kError:
			if (m_gaugeCalcType == GaugeCalcType::kCourseEasy || m_gaugeCalcType == GaugeCalcType::kCourseNormal || m_gaugeCalcType == GaugeCalcType::kCourseHard)
			{
				subtractGaugeValue(kGaugeDecreaseBaseValueCourseLongError);
			}
			else if (m_gaugeCalcType == GaugeCalcType::kNormalHard)
			{
				subtractGaugeValue(kGaugeDecreaseValueHardLongError);
			}
			else
			{
				subtractGaugeValue(static_cast<int32>(m_gaugeValueMax * kGaugeDecreasePercentByLongError / 100));
			}
			break;

		default:
			assert(false && "Invalid JudgmentResult in doLongJudgment");
			break;
		}
	}

	int32 ScoringStatus::score() const
	{
		if (m_scoreValueMax == 0)
		{
			return 0;
		}
		return static_cast<int32>(static_cast<int64>(kScoreMax) * m_scoreValue / m_scoreValueMax);
	}

	double ScoringStatus::calcGaugePercentageFromValue(int32 gaugeValue, GaugeType gaugeType) const
	{
		if (gaugeType == GaugeType::kHardGauge || m_gameMode == GameMode::kCourseMode)
		{
			return Max(0.0, 100.0 * gaugeValue / kGaugeValueMaxHard);
		}
		else
		{
			if (m_gaugeValueMax == 0)
			{
				return 0.0;
			}
			return 100.0 * gaugeValue / m_gaugeValueMax;
		}
	}

	double ScoringStatus::gaugePercentage(GaugeType gaugeType) const
	{
		return calcGaugePercentageFromValue(m_gaugeValue, gaugeType);
	}

	int32 ScoringStatus::gaugePercentageInt(GaugeType gaugeType) const
	{
		if (gaugeType == GaugeType::kHardGauge || m_gameMode == GameMode::kCourseMode)
		{
			return m_gaugeValue / 1000;
		}
		else
		{
			if (m_gaugeValueMax == 0)
			{
				return 0;
			}
			return 100 * m_gaugeValue / m_gaugeValueMax;
		}
	}

	double ScoringStatus::gaugePercentageForGrade() const
	{
		// HARDゲージまたはコースモードの場合はNORMAL基準のゲージ値を使用
		if (m_gaugeType == GaugeType::kHardGauge || m_gameMode == GameMode::kCourseMode)
		{
			return calcGaugePercentageFromValue(m_gaugeValueNormal, GaugeType::kNormalGauge);
		}
		else
		{
			return calcGaugePercentageFromValue(m_gaugeValue, m_gaugeType);
		}
	}

	int32 ScoringStatus::gaugeValue() const
	{
		return m_gaugeValue;
	}

	int32 ScoringStatus::combo() const
	{
		return m_comboStatus.combo();
	}

	int32 ScoringStatus::maxCombo() const
	{
		return m_comboStatus.maxCombo();
	}

	const ComboStats& ScoringStatus::comboStats() const
	{
		return m_comboStatus.stats();
	}

	bool ScoringStatus::isNoError() const
	{
		return m_comboStatus.isNoError();
	}

	int32 ScoringStatus::totalJudgedCombo() const
	{
		return m_comboStatus.totalJudgedCombo();
	}

	int32 ScoringStatus::courseCombo() const
	{
		return m_comboStatus.courseCombo();
	}

	bool ScoringStatus::courseIsNoError() const
	{
		return m_comboStatus.courseIsNoError();
	}

	int32 ScoringStatus::displayCombo() const
	{
		return m_gameMode == GameMode::kCourseMode ? m_comboStatus.courseCombo() : m_comboStatus.combo();
	}

	bool ScoringStatus::displayIsNoError() const
	{
		return m_gameMode == GameMode::kCourseMode ? m_comboStatus.courseIsNoError() : m_comboStatus.isNoError();
	}
}
