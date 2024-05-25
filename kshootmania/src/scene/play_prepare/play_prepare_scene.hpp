#pragma once
#include <CoTaskLib.hpp>
#include "ksmaudio/ksmaudio.hpp"
#include "play_prepare_assets.hpp"

class PlayPrepareScene : public Co::SceneBase
{
private:
	ksmaudio::Stream m_seStream{ "se/sel_enter.ogg", 1.0, false, false, false };

	const FilePath m_chartFilePath;

	const MusicGame::IsAutoPlayYN m_isAutoPlay;

	const kson::ChartData m_chartData;

	const Texture m_bgTexture{ TextureAsset(PlayPrepareTexture::kBG) };

	const Texture m_jacketTexture;

	double m_jacketScale = 1.0;

public:
	explicit PlayPrepareScene(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay);

	virtual ~PlayPrepareScene() = default;

	virtual Co::Task<void> start() override;

	virtual void draw() const override;

	virtual Co::Task<void> fadeIn() override;
};
