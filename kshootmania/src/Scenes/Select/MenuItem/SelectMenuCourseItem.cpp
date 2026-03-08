#include "SelectMenuCourseItem.hpp"
#include "Course/CoursePlayState.hpp"
#include "I18n/I18n.hpp"
#include "RuntimeConfig.hpp"
#include "HighScore/KscKey.hpp"
#include "HighScore/KscIO.hpp"
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
	KscIO::ReadAllCourseHighScoreInfo(courseInfo.filePath, &m_highScoreInfoMap);
}

void SelectMenuCourseItem::decide(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
{
	// コースの全譜面が存在するかチェック
	if (!m_courseInfo.isValid())
	{
		MessageBoxUtils::ShowOK(I18n::Get(I18n::Play::ErrorSomeChartMissingInCourse), MessageBoxStyle::Error);
		return;
	}

	// コースプレイ開始
	const FilePath firstChartPath = m_courseInfo.charts[0].absolutePath;
	if (!FileSystem::Exists(firstChartPath))
	{
		MessageBoxUtils::ShowOK(I18n::Get(I18n::Play::ErrorChartFileNotFound), MessageBoxStyle::Error);
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
		MessageBoxUtils::ShowOK(I18n::Get(I18n::Play::ErrorSomeChartMissingInCourse), MessageBoxStyle::Error);
		return;
	}

	// オートプレイでコースプレイ開始
	const FilePath firstChartPath = m_courseInfo.charts[0].absolutePath;
	if (!FileSystem::Exists(firstChartPath))
	{
		MessageBoxUtils::ShowOK(I18n::Get(I18n::Play::ErrorChartFileNotFound), MessageBoxStyle::Error);
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
	const HighScoreInfo info = highScoreInfo(0).value_or(HighScoreInfo{});
	const int32 medalIndex = static_cast<int32>(info.medal());
	const int32 achievementRate = info.percent(kscKey.gaugeType);

	// コース情報を設定
	canvas.setSubCanvasParamValuesByTag(U"center", {
		{ U"title", m_courseInfo.title },
		{ U"artist", U"COURSE ({} charts)"_fmt(m_courseInfo.chartCount()) },
		{ U"bpm", U"" },
		{ U"jacketAuthor", U"" },
		{ U"information", m_courseInfo.information },
		{ U"chartAuthor", U"" },
		{ U"difficultyCursorState", U"difficulty0" },
		{ U"medalIndex", medalIndex },
		{ U"highScoreGradeIndex", -1 },
		{ U"highScore", U"" },
		{ U"gaugePercentage", ToString(achievementRate) },
	});

	// Course用のノードを取得
	if (const auto courseNode = NocoUtils::GetSubCanvasNodeByName(&canvas, U"center", U"Course"))
	{
		// アイコン画像を設定
		if (const auto iconNode = courseNode->findByName(U"Icon"))
		{
			if (m_courseInfo.iconPath.isEmpty())
			{
				// アイコンが指定されていない場合は非表示
				iconNode->setActive(false);
			}
			else
			{
				// アイコンが指定されている場合はテクスチャロードして表示
				const Texture iconTexture = context.fnGetIconTexture(m_courseInfo.iconPath);
				iconNode->setActive(!iconTexture.isEmpty());
				if (const auto sprite = iconNode->getComponent<noco::Sprite>())
				{
					sprite->setTexture(iconTexture);
				}
			}
		}

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
				Optional<kson::MetaChartData> chartDataOpt;

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
						chartDataOpt = chartData;
					}
					else
					{
						Logger << U"[ksm warning] SelectMenuCourseItem: Chart meta loading failed (error:'{}', chartPath:'{}')"_fmt(
							Unicode::FromUTF8(kson::GetErrorString(chartData.error)),
							chart.absolutePath);
					}
				}

				const auto& node = chartItemRoot->addSubCanvasNodeAsChild(
					U"ui/parts/select_course_chart_item.noco",
					{
						{ U"songTitle", songTitle },
						{ U"artistName", artistName },
						{ U"difficultyIndex", chartDifficultyIdx },
						{ U"levelIndex", levelIdx },
					});

				if (const auto subCanvas = node->getComponent<noco::SubCanvas>())
				{
					if (auto itemCanvas = subCanvas->canvas())
					{
						if (const auto jacketSprite = NocoUtils::GetComponentByPath<noco::Sprite>(itemCanvas.get(), { U"SelectCourseChartItem", U"Jacket" }))
						{
							if (chartDataOpt.has_value() && !chartDataOpt->meta.jacketFilename.empty())
							{
								const FilePath jacketPath = FileSystem::PathAppend(
									FileSystem::ParentPath(chart.absolutePath),
									Unicode::FromUTF8(chartDataOpt->meta.jacketFilename));
								const Texture jacketTexture = context.fnGetJacketTexture(jacketPath);
								jacketSprite->setTexture(jacketTexture);
								if (jacketTexture.isEmpty())
								{
									jacketSprite->setColor(ColorF{ 0.0, 0.0 });
								}
								else
								{
									jacketSprite->setColor(Palette::White);
								}
							}
							else
							{
								jacketSprite->setColor(ColorF{ 0.0, 0.0 });
							}
						}
					}
				}
			}

			chartItemRoot->setVerticalScrollable(true);
			chartItemRoot->setScrollBarType(noco::ScrollBarType::Hidden);
			chartItemRoot->emplaceComponent<VerticalMarquee>(0.75s, 0.75s, 64.0);
		}
	}
}

void SelectMenuCourseItem::setCanvasParamsTopBottom([[maybe_unused]] const SelectMenuEventContext& context, noco::Canvas& canvas, [[maybe_unused]] int32 difficultyIdx, StringView tag) const
{
	// コースのハイスコア情報を取得
	const KscKey kscKey = CreateKscKeyFromConfig();
	const HighScoreInfo info = highScoreInfo(0).value_or(HighScoreInfo{});
	const int32 medalIndex = static_cast<int32>(info.medal());
	const int32 achievementRate = info.percent(kscKey.gaugeType);

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
		{ U"gaugePercentage", ToString(achievementRate) },
	});

	// Course用のノードを取得してアイコン画像を設定
	if (const auto courseNode = NocoUtils::GetSubCanvasNodeByName(&canvas, tag, U"Course"))
	{
		if (const auto iconNode = courseNode->findByName(U"Icon"))
		{
			if (m_courseInfo.iconPath.isEmpty())
			{
				// アイコンが指定されていない場合は非表示
				iconNode->setActive(false);
			}
			else
			{
				// アイコンが指定されている場合はテクスチャロードして表示
				const Texture iconTexture = context.fnGetIconTexture(m_courseInfo.iconPath);
				iconNode->setActive(!iconTexture.isEmpty());
				if (const auto sprite = iconNode->getComponent<noco::Sprite>())
				{
					sprite->setTexture(iconTexture);
				}
			}
		}
	}
}

void SelectMenuCourseItem::showInFileManager([[maybe_unused]] int32 difficultyIdx) const
{
	// コースファイルをエクスプローラで開く
	System::ShowInFileManager(m_courseInfo.filePath);
}

Optional<HighScoreInfo> SelectMenuCourseItem::highScoreInfo([[maybe_unused]] int32 difficultyIdx) const
{
	const String key = CreateKscKeyFromConfig().toStringWithoutGaugeType();
	if (auto it = m_highScoreInfoMap.find(key); it != m_highScoreInfoMap.end())
	{
		return it->second;
	}
	return HighScoreInfo{};
}
