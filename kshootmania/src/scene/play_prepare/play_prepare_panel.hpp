#pragma once
#include "graphics/number_texture_font.hpp"
#include "play_prepare_panel.hpp"
#include "kson/chart_data.hpp"

class PlayPreparePanel
{
private:
	const RenderTexture m_titlePanelBaseTexture;
	const Vec2 m_titlePanelPosition;

	const TiledTexture m_difficultyTexture;
	const TextureRegion m_difficultyTextureRegion;

	const int32 m_level;
	const double m_bpm;
	
	const NumberTextureFont m_numberTextureFont;
	const NumberTextureFont m_numberLargeTextureFont;
	const TextureFontTextLayout m_levelNumberLayout;
	const TextureFontTextLayout m_bpmNumberLayout;

public:
	PlayPreparePanel(FilePathView chartFilePath, const kson::ChartData& chartData);

	void draw() const;
};
