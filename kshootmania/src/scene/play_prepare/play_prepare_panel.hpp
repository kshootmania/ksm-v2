#pragma once
#include <CoTaskLib.hpp>
#include "play_prepare_assets.hpp"
#include "play_prepare_panel.hpp"

class PlayPreparePanel
{
private:
	const RenderTexture m_infoRenderTexture;

	const RenderTexture m_hispeedRenderTexture;

public:
	PlayPreparePanel(FilePathView chartFilePath, const kson::ChartData& chartData);

	void draw() const;
};
