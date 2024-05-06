#pragma once
#include <CoTaskLib.hpp>
#include "ksmaudio/ksmaudio.hpp"
#include "play_prepare_assets.hpp"

class PlayPrepareScene : public Co::SceneBase
{
private:
	ksmaudio::Stream m_seStream{ "se/sel_enter.ogg", 1.0, false, false, false };

	const Co::SceneFactory m_playSceneFactory;

	const Texture m_bgTexture{ TextureAsset(PlayPrepareTexture::kBG) };

public:
	explicit PlayPrepareScene(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay);

	virtual ~PlayPrepareScene() = default;

	virtual Co::Task<Co::SceneFactory> start() override;

	virtual void draw() const override;

	virtual Co::Task<void> fadeIn() override;
};
