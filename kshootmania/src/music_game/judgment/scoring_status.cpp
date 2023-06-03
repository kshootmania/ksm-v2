#include "scoring_status.hpp"

namespace MusicGame::Judgment
{
	void ScoringStatus::addGaugeValue(int32 add)
	{
		m_gaugeValue = Min(m_gaugeValue + add, m_gaugeValueMax);
	}

	void ScoringStatus::subtractGaugeValue(int32 sub)
	{
		m_gaugeValue = Max(m_gaugeValue - sub, 0);
	}

	ScoringStatus::ScoringStatus(int32 scoreValueMax, int32 gaugeValueMax)
		: m_scoreValueMax(scoreValueMax)
		, m_gaugeValueMax(gaugeValueMax)
	{
	}

	void ScoringStatus::doChipJudgment(Judgment::JudgmentResult result)
	{
		switch (result)
		{
		case Judgment::JudgmentResult::kCritical:
			m_scoreValue += Judgment::kScoreValueCritical;
			addGaugeValue(kGaugeValueChip);
			m_comboStatus.increment();
			break;

		case Judgment::JudgmentResult::kNear:
			m_scoreValue += Judgment::kScoreValueNear;
			addGaugeValue(kGaugeValueChipNear);
			m_comboStatus.increment();
			break;

		case Judgment::JudgmentResult::kError:
			subtractGaugeValue(static_cast<int32>(m_gaugeValueMax * kGaugeDecreasePercentByChipError / 100));
			m_comboStatus.resetByErrorJudgment();
			break;

		default:
			assert(false && "Invalid JudgmentResult in doChipJudgment");
			break;
		}
	}

	void ScoringStatus::doLongJudgment(Judgment::JudgmentResult result)
	{
		switch (result)
		{
		case Judgment::JudgmentResult::kCritical:
			m_scoreValue += Judgment::kScoreValueCritical;
			addGaugeValue(kGaugeValueLong);
			m_comboStatus.increment();
			break;

		case Judgment::JudgmentResult::kError:
			subtractGaugeValue(static_cast<int32>(m_gaugeValueMax * kGaugeDecreasePercentByLongError / 100));
			m_comboStatus.resetByErrorJudgment();
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

	double ScoringStatus::gaugePercentage() const
	{
		if (m_gaugeValueMax == 0)
		{
			return 0.0;
		}
		return 100.0 * m_gaugeValue / m_gaugeValueMax;
	}

	int32 ScoringStatus::combo() const
	{
		return m_comboStatus.combo();
	}

	bool ScoringStatus::isNoError() const
	{
		return m_comboStatus.isNoError();
	}
}
