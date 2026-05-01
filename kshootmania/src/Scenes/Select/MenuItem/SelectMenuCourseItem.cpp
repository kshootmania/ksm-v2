#include "SelectMenuCourseItem.hpp"
#include "Course/CoursePlayState.hpp"
#include "I18n/I18n.hpp"
#include "RuntimeConfig.hpp"
#include "HighScore/KscKey.hpp"
#include "NocoExtensions/NocoUtils.hpp"
#include "NocoExtensions/VerticalMarquee.hpp"

namespace
{
	// TODO: PlayOptionを選曲画面側で生成してそれを使うように
	KscKey CreateKscKeyFromConfig()
	{
		return KscKey
		{
			.gaugeType = RuntimeConfig::GetGaugeType(),
			.turnMode = RuntimeConfig::GetTurnMode(),
			.playbackSpeed = RuntimeConfig::GetPlaybackSpeed(),
			.btPlayMode = RuntimeConfig::GetJudgmentPlayModeBT(),
			.fxPlayMode = RuntimeConfig::GetJudgmentPlayModeFX(),
			.laserPlayMode = RuntimeConfig::GetJudgmentPlayModeLaser(),
		};
	}
}

SelectMenuCourseItem::SelectMenuCourseItem(const CourseInfo& courseInfo)
	: m_courseInfo(courseInfo)
{
}

void SelectMenuCourseItem::decide(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
{
	// コースの全譜面が存在するかチェック
	if (!m_courseInfo.isValid())
	{
		context.fnShowErrorDialog(I18n::Get(I18n::Play::ErrorSomeChartMissingInCourse));
		return;
	}

	// コースプレイ開始
	const FilePath firstChartPath = m_courseInfo.charts[0].absolutePath;
	if (!FileSystem::Exists(firstChartPath))
	{
		context.fnShowErrorDialog(I18n::Get(I18n::Play::ErrorChartFileNotFound));
		return;
	}

	// コース状態を初期化してPlaySceneに渡す
	const KscKey kscKey = CreateKscKeyFromConfig();
	CoursePlayState courseState{ m_courseInfo, kscKey };
	context.fnMoveToPlayScene(firstChartPath, MusicGame::IsAutoPlayYN::No, courseState);
}

void SelectMenuCourseItem::decideAutoPlay(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
{
	// コースの全譜面が存在するかチェック
	if (!m_courseInfo.isValid())
	{
		context.fnShowErrorDialog(I18n::Get(I18n::Play::ErrorSomeChartMissingInCourse));
		return;
	}

	// オートプレイでコースプレイ開始
	const FilePath firstChartPath = m_courseInfo.charts[0].absolutePath;
	if (!FileSystem::Exists(firstChartPath))
	{
		context.fnShowErrorDialog(I18n::Get(I18n::Play::ErrorChartFileNotFound));
		return;
	}

	// コース状態を初期化してPlaySceneに渡す
	const KscKey kscKey = CreateKscKeyFromConfig();
	CoursePlayState courseState{ m_courseInfo, kscKey };
	context.fnMoveToPlayScene(firstChartPath, MusicGame::IsAutoPlayYN::Yes, courseState);
}

void SelectMenuCourseItem::setCanvasParamsCenter(const SelectMenuEventContext& context, noco::Canvas& canvas, [[maybe_unused]] int32 difficultyIdx) const
{
	canvas.setSubCanvasParamValuesByTag(U"center", {
		{ U"isSong", false },
		{ U"isDirectory", false },
		{ U"isSubDirectory", false },
		{ U"isCourse", true },
	});

	// 難易度は非表示
	for (int32 i = 0; i < kNumDifficulties; ++i)
	{
		canvas.setSubCanvasParamValuesByTag(U"center", {
			{ U"difficulty{}Enabled"_fmt(i), false },
			{ U"difficulty{}LevelIndex"_fmt(i), -1 },
		});
	}

	// コースのハイスコア情報を取得
	const KscKey kscKey = CreateKscKeyFromConfig();
	const HighScoreInfo info = highScoreInfo(context, 0).value_or(HighScoreInfo{});
	const int32 medalIndex = static_cast<int32>(info.medal());
	const int32 achievementRate = info.percent(kscKey.gaugeType);

	// コース情報を設定
	const FilePath iconPath = m_courseInfo.iconPath;

	canvas.setSubCanvasParamValuesByTag(U"center", {
		{ U"title", m_courseInfo.title },
		{ U"artist", U"COURSE ({} charts)"_fmt(m_courseInfo.chartCount()) },
		{ U"bpm", U"" },
		{ U"jacketAuthor", U"" },
		{ U"information", m_courseInfo.information },
		{ U"chartAuthor", U"" },
		{ U"difficultyIndex", 0 },
		{ U"medalIndex", medalIndex },
		{ U"highScoreGradeIndex", -1 },
		{ U"highScore", 0 },
		{ U"gaugePercentage", achievementRate },
		{ U"iconFilePath", iconPath },
		{ U"iconActive", !iconPath.isEmpty() && FileSystem::IsFile(iconPath) },
	});

	// Course用のノードを取得
	if (const auto courseNode = NocoUtils::GetSubCanvasNodeByName(&canvas, U"center", U"Course"))
	{
		// コースの各曲をSubCanvasノードとして追加
		if (const auto chartItemRoot = courseNode->findByName(U"ChartItemRoot"))
		{
			chartItemRoot->removeChildrenAll();
			chartItemRoot->removeComponents<VerticalMarquee>(noco::RecursiveYN::No);

			// 各曲のSubCanvasノードを追加
			for (size_t i = 0; i < m_courseInfo.charts.size(); ++i)
			{
				const auto& chart = m_courseInfo.charts[i];

				String songTitle = U"---";
				String artistName = U"---";
				int32 chartDifficultyIdx = 0;
				int32 levelIdx = 0;
				FilePath chartJacketPath;
				bool chartJacketActive = false;
				FilePath songTitleImgPath;
				FilePath artistNameImgPath;

				if (FileSystem::Exists(chart.absolutePath))
				{
					const kson::MetaChartData chartData = FsUtils::HasKsonExtension(chart.absolutePath)
						? kson::LoadKsonMetaChartData(chart.absolutePath.toUTF8())
						: kson::LoadKshMetaChartData(chart.absolutePath.toUTF8());

					if (chartData.error == kson::ErrorType::None)
					{
						songTitle = Unicode::FromUTF8(chartData.meta.title);
						artistName = Unicode::FromUTF8(chartData.meta.artist);
						chartDifficultyIdx = chartData.meta.difficulty.idx;
						levelIdx = chartData.meta.level - 1;

						const FilePath chartDir = FileSystem::ParentPath(chart.absolutePath);

						if (!chartData.meta.jacketFilename.empty())
						{
							chartJacketPath = FsUtils::ResolveJacketPath(
								chartDir,
								Unicode::FromUTF8(chartData.meta.jacketFilename));
							chartJacketActive = !chartJacketPath.isEmpty() && FileSystem::IsFile(chartJacketPath);
						}

						if (!chartData.meta.titleImgFilename.empty())
						{
							songTitleImgPath = FileSystem::PathAppend(chartDir, Unicode::FromUTF8(chartData.meta.titleImgFilename));
							if (FileSystem::IsFile(songTitleImgPath))
							{
								songTitle = U"";
							}
							else
							{
								songTitleImgPath.clear();
							}
						}

						if (!chartData.meta.artistImgFilename.empty())
						{
							artistNameImgPath = FileSystem::PathAppend(chartDir, Unicode::FromUTF8(chartData.meta.artistImgFilename));
							if (FileSystem::IsFile(artistNameImgPath))
							{
								artistName = U"";
							}
							else
							{
								artistNameImgPath.clear();
							}
						}
					}
					else
					{
						Logger << U"[ksm warning] SelectMenuCourseItem: Chart meta loading failed (error:'{}', chartPath:'{}')"_fmt(
							Unicode::FromUTF8(kson::GetErrorString(chartData.error)),
							chart.absolutePath);
					}
				}

				chartItemRoot->addSubCanvasNodeAsChild(
					U"ui/parts/select_course_chart_item.noco",
					{
						{ U"songTitle", songTitle },
						{ U"songTitleImgFilePath", songTitleImgPath },
						{ U"artistName", artistName },
						{ U"artistNameImgFilePath", artistNameImgPath },
						{ U"difficultyIndex", chartDifficultyIdx },
						{ U"levelIndex", levelIdx },
						{ U"jacketFilePath", chartJacketPath },
						{ U"jacketActive", chartJacketActive },
					});
			}

			chartItemRoot->setVerticalScrollable(true);
			chartItemRoot->setScrollBarType(noco::ScrollBarType::Hidden);
			chartItemRoot->emplaceComponent<VerticalMarquee>(0.75s, 0.75s, 64.0);
		}
	}
}

void SelectMenuCourseItem::setCanvasParamsTopBottom(const SelectMenuEventContext& context, noco::Canvas& canvas, [[maybe_unused]] int32 difficultyIdx, StringView tag) const
{
	// コースのハイスコア情報を取得
	const KscKey kscKey = CreateKscKeyFromConfig();
	const HighScoreInfo info = highScoreInfo(context, 0).value_or(HighScoreInfo{});
	const int32 medalIndex = static_cast<int32>(info.medal());
	const int32 achievementRate = info.percent(kscKey.gaugeType);

	const FilePath iconPath = m_courseInfo.iconPath;

	canvas.setSubCanvasParamValuesByTag(tag, {
		{ U"isSong", false },
		{ U"isDirectory", false },
		{ U"isSubDirectory", false },
		{ U"isCourse", true },
		{ U"title", m_courseInfo.title },
		{ U"artist", U"" },
		{ U"levelIndex", -1 },
		{ U"medalIndex", medalIndex },
		{ U"highScoreGradeIndex", -1 },
		{ U"gaugePercentage", achievementRate },
		{ U"iconFilePath", iconPath },
		{ U"iconActive", !iconPath.isEmpty() && FileSystem::IsFile(iconPath) },
	});
}

void SelectMenuCourseItem::showInFileManager([[maybe_unused]] int32 difficultyIdx) const
{
	// コースファイルをエクスプローラで開く
	System::ShowInFileManager(m_courseInfo.filePath);
}

Optional<HighScoreInfo> SelectMenuCourseItem::highScoreInfo(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx) const
{
	return context.fnGetCourseHighScore(m_courseInfo.filePath);
}
