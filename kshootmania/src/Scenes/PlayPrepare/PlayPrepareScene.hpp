#pragma once
#include <CoTaskLib.hpp>
#include "PlayPrepareAssets.hpp"
#include "MusicGame/UI/HispeedSettingMenu.hpp"
#include "MusicGame/Scroll/HighwayScroll.hpp"
#include "Course/CoursePlayState.hpp"

class PlayPrepareScene : public Co::SceneBase
{
private:
	const FilePath m_chartFilePath;

	const MusicGame::IsAutoPlayYN m_isAutoPlay;

	const kson::ChartData m_chartData;

	Optional<CoursePlayState> m_courseState;

	Optional<MusicGame::TestPlayOption> m_testPlayOption;

	std::shared_ptr<noco::Canvas> m_canvas;

	MusicGame::HispeedSettingMenu m_hispeedMenu;

	MusicGame::Scroll::HighwayScroll m_highwayScroll;

	Stopwatch m_stopwatchSinceHispeedChange{ StartImmediately::Yes };

public:
	explicit PlayPrepareScene(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay, const Optional<CoursePlayState>& courseState = none, const Optional<MusicGame::TestPlayOption>& testPlayOption = none);

	virtual ~PlayPrepareScene() = default;

	virtual Co::Task<void> start() override;

	void update();

	virtual void draw() const override;

	virtual Co::Task<void> fadeIn() override;

	virtual Co::Task<void> postFadeOut() override;
};
