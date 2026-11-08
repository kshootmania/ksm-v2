#include "CoursePlayState.hpp"

CoursePlayResult CoursePlayState::coursePlayResult() const
{
	const int32 avgScore = totalChartCount() > 0 ? static_cast<int32>(m_totalScore / totalChartCount()) : 0;
	const int32 rate = achievementRate();

	return CoursePlayResult
	{
		.avgScore = avgScore,
		.maxCombo = m_courseMaxCombo,
		.achievementRate = rate,
		.gaugeType = m_kscKey.gaugeType,
		.isCleared = m_cleared,
		.results = m_results,
	};
}
