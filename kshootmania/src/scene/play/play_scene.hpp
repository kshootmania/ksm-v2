#pragma once
#include <CoTaskLib.hpp>
#include "music_game/game_main.hpp"

class PlayScene : public Co::UpdateSceneBase
{
private:
	/// @brief ゲームのメインクラス
	MusicGame::GameMain m_gameMain;

	/// @brief オートプレイかどうか
	MusicGame::IsAutoPlayYN m_isAutoPlay;

	Duration m_fadeOutDuration;

public:
	explicit PlayScene(FilePathView filePath, MusicGame::IsAutoPlayYN isAutoPlay);

	virtual ~PlayScene();

	virtual void update() override;

	virtual void draw() const override;

	virtual Co::Task<void> fadeIn() override;

	virtual Co::Task<void> fadeOut() override;
};
