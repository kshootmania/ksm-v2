﻿#pragma once
#include "result_assets.hpp"
#include "result_panel.hpp"
#include "music_game/play_result.hpp"

class ResultScene : public MyScene
{
private:
	const Texture m_bgTexture{ TextureAsset(ResultTexture::kBG) };

	const kson::ChartData m_chartData;

	const MusicGame::PlayResult m_playResult;

	ResultPanel m_resultPanel;

public:
	explicit ResultScene(const InitData& initData);

	virtual ~ResultScene() = default;

	virtual void update() override;

	virtual void updateFadeIn([[maybe_unused]] double t) override;

	virtual void draw() const override;

	virtual void drawFadeIn(double t) const override;
};
