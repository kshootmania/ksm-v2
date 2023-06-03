#include "combo_status.hpp"

namespace MusicGame::Judgment
{
	void ComboStatus::increment()
	{
		++m_combo;
	}

	void ComboStatus::resetByErrorJudgment()
	{
		m_combo = 0;
		m_isNoError = false;
	}

	int32 ComboStatus::combo() const
	{
		return m_combo;
	}

	bool ComboStatus::isNoError() const
	{
		return m_isNoError;
	}
}
