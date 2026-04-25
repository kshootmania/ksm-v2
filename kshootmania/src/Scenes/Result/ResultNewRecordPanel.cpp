#include "ResultNewRecordPanel.hpp"
#include "NocoExtensions/NocoUtils.hpp"

ResultNewRecordPanel::ResultNewRecordPanel(std::shared_ptr<noco::Canvas> canvas)
	: m_canvas(canvas)
{
}

void ResultNewRecordPanel::setValue(int32 oldScore, int32 newScore)
{
	const bool shouldBeVisible = newScore > oldScore;

	if (shouldBeVisible)
	{
		const int32 scoreDelta = newScore - oldScore;
		m_canvas->setParamValue(U"overlay_newRecordNumber", scoreDelta);
	}

	m_isVisible = shouldBeVisible;
}

bool ResultNewRecordPanel::isVisible() const
{
	return m_isVisible;
}

void ResultNewRecordPanel::startDisplay()
{
	m_canvas->setParamValue(U"overlay_newRecordPanelVisible", true);
	m_canvas->setTweenActiveByTag(U"out_newRecord", false);
	m_canvas->setTweenActiveByTag(U"in_newRecord", true);
}

void ResultNewRecordPanel::startRedisplay()
{
	m_canvas->setParamValue(U"overlay_newRecordPanelVisible", true);
	m_canvas->setTweenActiveByTag(U"out_newRecord", false);
	m_canvas->setTweenActiveByTag(U"inAgain_newRecord", true);
}

Co::Task<void> ResultNewRecordPanel::waitForFadeIn()
{
	co_await NocoUtils::WaitForTweenByTag(m_canvas, U"in_newRecord");
	co_await NocoUtils::WaitForTweenByTag(m_canvas, U"inAgain_newRecord");
}

void ResultNewRecordPanel::startFadeOut()
{
	m_canvas->setTweenActiveByTag(U"in_newRecord", false);
	m_canvas->setTweenActiveByTag(U"inAgain_newRecord", false);
	m_canvas->setTweenActiveByTag(U"out_newRecord", true);
}

Co::Task<void> ResultNewRecordPanel::waitForFadeOut()
{
	co_await NocoUtils::WaitForTweenByTag(m_canvas, U"out_newRecord");

	m_canvas->setParamValue(U"overlay_newRecordPanelVisible", false);
	m_isVisible = false;
}
