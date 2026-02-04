#pragma once
#include <CoTaskLib.hpp>
#include "MusicGame/GameMain.hpp"
#include "Course/CoursePlayState.hpp"

class PlayScene : public Co::UpdaterSceneBase
{
private:
	/// @brief ゲームのメインクラス
	MusicGame::GameMain m_gameMain;

	/// @brief オートプレイかどうか
	MusicGame::IsAutoPlayYN m_isAutoPlay;

	/// @brief コース状態(コースモード時のみ有効)
	Optional<CoursePlayState> m_courseState;

	Duration m_fadeOutDuration;

	/// @brief テストプレイオプション
	Optional<MusicGame::TestPlayOption> m_testPlayOption;

	bool m_backButtonPressedDuringFadeOut = false;

	void updateFadeOut();

	void processBackButtonInput();

public:
	explicit PlayScene(FilePathView filePath, MusicGame::IsAutoPlayYN isAutoPlay, const Optional<CoursePlayState>& courseState = none, const Optional<MusicGame::TestPlayOption>& testPlayOption = none);

	virtual ~PlayScene();

	virtual void update() override;

	virtual void draw() const override;

	virtual Co::Task<void> fadeIn() override;

	virtual Co::Task<void> fadeOut() override;
};
