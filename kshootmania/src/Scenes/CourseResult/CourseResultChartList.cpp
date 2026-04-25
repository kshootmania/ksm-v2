#include "CourseResultChartList.hpp"
#include "Input/Cursor/CursorInput.hpp"
#include "Course/CoursePlayState.hpp"
#include "Scenes/Select/SelectChartInfo.hpp"

namespace
{
	int32 GradeToIndex(Grade grade)
	{
		assert(grade != Grade::kNoGrade && "Unexpected grade in play result: Grade::kNoGrade");
		return static_cast<int32>(grade) - 1;
	}
}

CourseResultChartList::CourseResultChartList(std::shared_ptr<noco::Canvas> canvas, const CoursePlayState& courseState)
	: m_canvas(canvas)
	, m_pageMenu(LinearMenu::CreateInfoWithCursorMinMax{
		.cursorInputCreateInfo = CursorInput::CreateInfo{
			.type = CursorInput::Type::Vertical,
			.buttonFlags = CursorButtonFlags::kArrowOrBT,
			.buttonIntervalSec = 0.0,
		},
		.cursorMin = 0,
		.cursorMax = Max(0, (courseState.courseInfo().chartCount() + kItemsPerPage - 1) / kItemsPerPage - 1),
		.defaultCursor = 0,
	})
{
	refreshCanvasParams(courseState);
}

void CourseResultChartList::update(const CoursePlayState& courseState)
{
	m_pageMenu.update();

	if (m_pageMenu.deltaCursor() != 0)
	{
		refreshCanvasParams(courseState);
		m_canvas->setTweenActiveByTag(U"onScroll", false);
		m_canvas->setTweenActiveByTag(U"onScroll", true);
	}
}

int32 CourseResultChartList::currentPage() const
{
	return m_pageMenu.cursor();
}

bool CourseResultChartList::shouldShowArrows() const
{
	const int32 totalPages = m_pageMenu.cursor() == 0 ? 1 : (m_pageMenu.cursor() + 1);
	return totalPages > 1;
}

void CourseResultChartList::refreshCanvasParams(const CoursePlayState& courseState)
{
	const int32 currentPage = m_pageMenu.cursor();
	const int32 startIdx = currentPage * kItemsPerPage;
	const CourseInfo& courseInfo = courseState.courseInfo();
	const Array<MusicGame::PlayResult>& results = courseState.results();
	const int32 totalItemCount = courseInfo.chartCount();

	// 矢印の表示制御
	const bool showArrows = totalItemCount > kItemsPerPage;
	m_canvas->setParamValue(U"arrowUpVisible", showArrows && !m_pageMenu.isCursorMin());
	m_canvas->setParamValue(U"arrowDownVisible", showArrows && !m_pageMenu.isCursorMax());

	// 各アイテムの表示制御とパラメータ設定
	for (int32 i = 0; i < kItemsPerPage; ++i)
	{
		const int32 chartIdx = startIdx + i;
		const bool visible = chartIdx < totalItemCount;

		m_canvas->setParamValue(U"chartItem{}Visible"_fmt(i + 1), visible);

		if (!visible)
		{
			continue;
		}

		if (chartIdx >= static_cast<int32>(results.size()))
		{
			Logger << U"[ksm error] Course result index mismatch: chartIdx={}, results.size()={}"_fmt(chartIdx, results.size());
			continue;
		}

		const FilePath& chartPath = courseInfo.charts[chartIdx].absolutePath;
		const MusicGame::PlayResult& result = results[chartIdx];

		if (!FileSystem::Exists(chartPath))
		{
			Logger << U"[ksm error] Chart file not found: {}"_fmt(chartPath);
			m_canvas->setSubCanvasParamValuesByTag(U"chartItem{}"_fmt(i + 1), {
				{ U"title", U"---" },
				{ U"artist", U"---" },
				{ U"jacketFilePath", U"" },
				{ U"difficultyIndex", 0 },
				{ U"levelNumber", 0 },
				{ U"gradeIndex", static_cast<double>(GradeToIndex(result.grade())) },
				{ U"scoreNumber", result.score },
			});
			continue;
		}

		const SelectChartInfo chartInfo{ chartPath };

		const FilePath titleImgPath = chartInfo.titleImgFilePath();
		const bool hasTitleImg = !titleImgPath.isEmpty() && FileSystem::IsFile(titleImgPath);
		const FilePath artistImgPath = chartInfo.artistImgFilePath();
		const bool hasArtistImg = !artistImgPath.isEmpty() && FileSystem::IsFile(artistImgPath);

		m_canvas->setSubCanvasParamValuesByTag(U"chartItem{}"_fmt(i + 1), {
			{ U"title", hasTitleImg ? U"" : chartInfo.title() },
			{ U"titleImgFilePath", titleImgPath },
			{ U"artist", hasArtistImg ? U"" : chartInfo.artist() },
			{ U"artistImgFilePath", artistImgPath },
			{ U"jacketFilePath", chartInfo.jacketFilePath() },
			{ U"difficultyIndex", static_cast<double>(chartInfo.difficultyIdx()) },
			{ U"levelNumber", chartInfo.level() },
			{ U"gradeIndex", static_cast<double>(GradeToIndex(result.grade())) },
			{ U"scoreNumber", result.score },
		});
	}
}
