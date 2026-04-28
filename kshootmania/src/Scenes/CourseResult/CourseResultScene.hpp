#pragma once
#include <CoTaskLib.hpp>
#include "Course/CoursePlayState.hpp"
#include "Scenes/CourseResult/CourseResultNewRecordPanel.hpp"
#include "Scenes/CourseResult/CourseResultChartList.hpp"
#include "Scenes/Result/ResultSNSShare.hpp"
#include "Scenes/Select/SelectSceneSearchParams.hpp"
#include "ksmaudio/ksmaudio.hpp"

class CourseResultScene : public Co::SceneBase
{
private:
	ksmaudio::Stream m_bgmStream{ "se/result_bgm.ogg", 1.0, false, false, true };

	std::shared_ptr<noco::Canvas> m_canvas;

	const CoursePlayState m_courseState;

	Optional<SelectSceneSearchParams> m_selectSearchParams;

	CourseResultNewRecordPanel m_newRecordPanel;

	CourseResultChartList m_chartList;

	ResultSNSShare m_snsShare;

	void updateCanvasParams();

	void update();

	Co::Task<bool> waitForNewRecordPanelClose();

public:
	explicit CourseResultScene(const CoursePlayState& courseState, const Optional<SelectSceneSearchParams>& selectSearchParams = none);

	virtual ~CourseResultScene() = default;

	virtual Co::Task<void> start() override;

	virtual void draw() const override;

	virtual Co::Task<void> fadeIn() override;

	virtual Co::Task<void> postFadeOut() override;
};
