#include "CourseResultNewRecordPanel.hpp"
#include "NocoExtensions/NocoUtils.hpp"

CourseResultNewRecordPanel::CourseResultNewRecordPanel(std::shared_ptr<noco::Canvas> canvas)
	: m_canvas(canvas)
{
}

void CourseResultNewRecordPanel::setValue(int32 oldAchievementRate, int32 newAchievementRate)
{
	const bool shouldBeVisible = newAchievementRate > oldAchievementRate;

	if (shouldBeVisible)
	{
		m_canvas->setParamValue(U"overlay_newRecordAchievementRateBefore", oldAchievementRate);
		m_canvas->setParamValue(U"overlay_newRecordAchievementRateAfter", newAchievementRate);
	}

	m_isVisible = shouldBeVisible;
}

bool CourseResultNewRecordPanel::isVisible() const
{
	return m_isVisible;
}

void CourseResultNewRecordPanel::startDisplay()
{
	m_canvas->setParamValue(U"overlay_newRecordPanelVisible", true);
	m_canvas->setTweenActiveByTag(U"out_newRecord", false);
	m_canvas->setTweenActiveByTag(U"in_newRecord", true);
}

void CourseResultNewRecordPanel::startRedisplay()
{
	m_canvas->setParamValue(U"overlay_newRecordPanelVisible", true);
	m_canvas->setTweenActiveByTag(U"out_newRecord", false);
	m_canvas->setTweenActiveByTag(U"inAgain_newRecord", true);
}

Co::Task<void> CourseResultNewRecordPanel::waitForFadeIn()
{
	co_await NocoUtils::WaitForTweenByTag(m_canvas, U"in_newRecord");
	co_await NocoUtils::WaitForTweenByTag(m_canvas, U"inAgain_newRecord");
}

void CourseResultNewRecordPanel::startFadeOut()
{
	m_canvas->setTweenActiveByTag(U"in_newRecord", false);
	m_canvas->setTweenActiveByTag(U"inAgain_newRecord", false);
	m_canvas->setTweenActiveByTag(U"out_newRecord", true);
}

Co::Task<void> CourseResultNewRecordPanel::waitForFadeOut()
{
	co_await NocoUtils::WaitForTweenByTag(m_canvas, U"out_newRecord");

	m_canvas->setParamValue(U"overlay_newRecordPanelVisible", false);
	m_isVisible = false;
}
