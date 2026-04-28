#pragma once
#include <CoTaskLib.hpp>
#include "SelectMenu.hpp"
#include "SelectFolderState.hpp"
#include "SelectSceneSearchParams.hpp"
#include "Input/FXButtonUpDetection.hpp"
#include "BTOptionPanel.hpp"
#include "PlayStatsPanel.hpp"
#include "FavoriteAddDialog.hpp"
#include "FavoriteRemoveDialog.hpp"

class SelectScene : public Co::UpdaterSceneBase
{
private:
	const Button m_folderCloseButton;

	ColorF m_fadeOutColor = Palette::Black;

	bool m_skipFadeout = false;

	std::shared_ptr<noco::Canvas> m_canvas;

	std::shared_ptr<noco::Node> m_searchResultNode;

	SelectSceneSearchPhase m_searchPhase = SelectSceneSearchPhase::kNone;

	String m_searchResultQuery;

	bool m_searchInputReentryFromResult = false;

	SelectMenu m_menu;

	Array<String> m_playerNames;

	FXButtonUpDetection m_fxButtonUpDetection;

	BTOptionPanel m_btOptionPanel;

	PlayStatsPanel m_playStatsPanel;

	Optional<Co::ScopedTaskRunner> m_dialogRunner;

	// Startボタン長押し検出
	Stopwatch m_startKeyPressStopwatch;

	// 次回のStartボタンの離した判定を無視するか
	bool m_ignoreNextStartUp = false;

	void moveToPlayScene(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay, const Optional<CoursePlayState>& courseState = none);

	void refreshCanvasPlayerName();

	void updatePlayerSwitching();

	void updateAlphabetJump();

	void updateStartKeyLongPress();

	void enterSearchInputPhase();

	void exitSearchMode();

	void onSearchInputDialogClosed(const Optional<String>& result);

public:
	explicit SelectScene(const Optional<SelectSceneSearchParams>& initialSearchParams = none);

	virtual void update() override;

	virtual void draw() const override;

	virtual Co::Task<void> fadeIn() override;

	virtual Co::Task<void> fadeOut() override;
};
