#include "combo_status.hpp"

namespace MusicGame::Judgment
{
	void ComboStatus::processJudgmentResult(JudgmentResult result)
	{
		switch (result)
		{
		case JudgmentResult::kCritical:
			++m_combo;
			m_maxCombo = Max(m_maxCombo, m_combo);
			++m_stats.critical;
			break;

		case JudgmentResult::kNearFast:
			++m_combo;
			m_maxCombo = Max(m_maxCombo, m_combo);
			++m_stats.nearFast;
			break;

		case JudgmentResult::kNearSlow:
			++m_combo;
			m_maxCombo = Max(m_maxCombo, m_combo);
			++m_stats.nearSlow;
			break;

		case JudgmentResult::kError:
			m_combo = 0;
			++m_stats.error;
			break;

		default:
			assert(false && "Invalid JudgmentResult in processJudgmentResult");
			break;
		}
	}

	int32 ComboStatus::combo() const
	{
		return m_combo;
	}

	int32 ComboStatus::maxCombo() const
	{
		return m_maxCombo;
	}

	const ComboStats& ComboStatus::stats() const
	{
		return m_stats;
	}

	bool ComboStatus::isNoError() const
	{
		return m_stats.error == 0;
	}
}
