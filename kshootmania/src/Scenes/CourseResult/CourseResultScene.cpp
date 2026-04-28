#include "CourseResultScene.hpp"
#include "Scenes/Select/SelectScene.hpp"
#include "Scenes/Select/SelectChartInfo.hpp"
#include "Scenes/Common/ShowLoadingOneFrame.hpp"
#include "Addon/AutoMuteAddon.hpp"
#include "Common/FsUtils.hpp"
#include "Input/KeyConfig.hpp"
#include "HighScore/KscIO.hpp"

namespace
{
	constexpr Duration kFadeDuration = 0.6s;
	constexpr double kGaugeBarMaxWidth = 540.0;

	FilePath GetCourseResultSceneUIFilePath()
	{
		return FsUtils::GetResourcePath(U"ui/scene/course_result.noco");
	}

	std::shared_ptr<noco::Canvas> LoadCourseResultSceneCanvas()
	{
		const FilePath uiFilePath = GetCourseResultSceneUIFilePath();
		const auto canvas = noco::Canvas::LoadFromFile(uiFilePath);
		if (!canvas)
		{
			throw Error{ U"Failed to load '{}'"_fmt(uiFilePath) };
		}
		return canvas;
	}

	String BuildCourseTweetText(const CoursePlayState& courseState)
	{
		const StringView clearStatus = courseState.isCleared() ? U"CLEARED!" : U"FAILED...";
		const int32 achievementRate = courseState.achievementRate();

		return U"{}\n{} (ACHIEVEMENT RATE: {}%)\n#kshootmania"_fmt(
			courseState.courseInfo().title,
			clearStatus,
			achievementRate);
	}
}

CourseResultScene::CourseResultScene(const CoursePlayState& courseState, const Optional<SelectSceneSearchParams>& selectSearchParams)
	: m_canvas(LoadCourseResultSceneCanvas())
	, m_courseState(courseState)
	, m_selectSearchParams(selectSearchParams)
	, m_newRecordPanel(m_canvas)
	, m_chartList(m_canvas, m_courseState)
	, m_snsShare(BuildCourseTweetText(m_courseState))
{
	// 前回までのAchievementRateを読み込んでNewRecordパネルを設定
	const FilePath courseFilePath = m_courseState.courseInfo().filePath;
	const KscKey kscKey = m_courseState.kscKey();

	// スコア保存前に前回までのAchievementRateを取得
	const HighScoreInfo oldHighScore = KscIO::ReadCourseHighScoreInfo(courseFilePath, kscKey);
	const int32 oldAchievementRate = oldHighScore.percent(kscKey.gaugeType);

	// NewRecordパネルに値設定
	const int32 newAchievementRate = m_courseState.achievementRate();
	m_newRecordPanel.setValue(oldAchievementRate, newAchievementRate);

	// スコアを保存
	KscIO::WriteCourseHighScoreInfo(courseFilePath, m_courseState);

	updateCanvasParams();

	m_bgmStream.play();
	AutoMuteAddon::SetEnabled(true);
}

void CourseResultScene::updateCanvasParams()
{
	const CourseInfo& courseInfo = m_courseState.courseInfo();
	const int32 newAchievementRate = m_courseState.achievementRate();

	m_canvas->setParamValues({
		{ U"courseTitle", courseInfo.title },
		{ U"resultTopIndex", m_courseState.isCleared() ? 1.0 : 0.0 },
		{ U"gaugePercentageNumber", newAchievementRate },
		{ U"gaugeTextureIndex", 1.0 },
		{ U"bottomRightText", U"" },
	});

	// ゲージのバーの幅をパーセンテージに応じて変更
	if (auto gaugeClippingMask = m_canvas->findByName(U"GaugeClippingMask"))
	{
		if (const auto* inlineRegion = gaugeClippingMask->inlineRegion())
		{
			noco::InlineRegion newRegion = *inlineRegion;
			const double gaugeRatio = Clamp(newAchievementRate / 100.0, 0.0, 1.0);
			newRegion.sizeDelta.x = kGaugeBarMaxWidth * gaugeRatio;
			gaugeClippingMask->setRegion(newRegion);
		}
	}
}

Co::Task<void> CourseResultScene::start()
{
	const auto updateRunner = Co::UpdaterTask([this] { update(); }).runScoped();

	bool userPressedStartOrBack = false;

	if (m_newRecordPanel.isVisible())
	{
		m_newRecordPanel.startDisplay();
		co_await m_newRecordPanel.waitForFadeIn();

		while (true)
		{
			const bool shouldClose = co_await waitForNewRecordPanelClose();

			if (shouldClose)
			{
				userPressedStartOrBack = true;
				break;
			}

			// FX-L+R同時押しで再表示
			m_newRecordPanel.startRedisplay();
			co_await m_newRecordPanel.waitForFadeIn();
		}
	}

	if (!userPressedStartOrBack)
	{
		while (true)
		{
			co_await Co::NextFrame();
			if (KeyConfig::Down(kButtonBack))
			{
				break;
			}
			if (KeyConfig::Down(kButtonStart) && !KeyShift.pressed())
			{
				break;
			}
		}
	}

	requestNextScene<SelectScene>(m_selectSearchParams);
}

Co::Task<bool> CourseResultScene::waitForNewRecordPanelClose()
{
	bool fxLRPressedPrev = false;
	Stopwatch displayStopwatch{ StartImmediately::Yes };

	while (true)
	{
		co_await Co::NextFrame();

		// FX-L+R同時押しで表示時間を3秒延長
		const bool fxLRPressed = KeyConfig::Pressed(kButtonFX_L) && KeyConfig::Pressed(kButtonFX_R);
		if (fxLRPressed && !fxLRPressedPrev)
		{
			displayStopwatch.restart();
		}
		fxLRPressedPrev = fxLRPressed;

		// 3秒経過またはSTART/Backで終了(Shift+STARTはツイート機能用なので除外)
		if (displayStopwatch.sF() >= 3.0 ||
			(KeyConfig::Down(kButtonStart) && !KeyShift.pressed()) ||
			KeyConfig::Down(kButtonBack))
		{
			break;
		}
	}

	m_newRecordPanel.startFadeOut();
	co_await m_newRecordPanel.waitForFadeOut();

	// フェードアウト後、FX-L+Rで再表示
	while (true)
	{
		co_await Co::NextFrame();

		const bool fxLRPressed = KeyConfig::Pressed(kButtonFX_L) && KeyConfig::Pressed(kButtonFX_R);
		if (fxLRPressed && !fxLRPressedPrev)
		{
			co_return false;
		}
		fxLRPressedPrev = fxLRPressed;

		if ((KeyConfig::Down(kButtonStart) && !KeyShift.pressed()) || KeyConfig::Down(kButtonBack))
		{
			co_return true;
		}
	}
}

void CourseResultScene::update()
{
	m_canvas->update();
	m_chartList.update(m_courseState);
	m_snsShare.update(m_canvas.get());
}

void CourseResultScene::draw() const
{
	m_canvas->draw();
}

Co::Task<void> CourseResultScene::fadeIn()
{
	const auto canvasUpdateRunner = Co::UpdaterTask([this] { m_canvas->update(); }).runScoped();

	co_await Co::ScreenFadeIn(kFadeDuration, Palette::White);
}

Co::Task<void> CourseResultScene::postFadeOut()
{
	// SelectSceneはコンストラクタの処理に時間がかかるので、ローディングはここで出しておく
	return ShowLoadingOneFrame::Play(LoadingBgMode::kMainBg);
}
