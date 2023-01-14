#pragma once
#include "graphics/number_texture_font.hpp"
#include "music_game/scroll/highway_scroll.hpp"
#include "hispeed_setting_panel.hpp"
#include "kson/chart_data.hpp"

namespace MusicGame::Graphics
{
	class SongInfoPanel
	{
	private:
		const Texture m_jacketTexture;
		const Vec2 m_jacketPosition;
		const SizeF m_scaledJacketSize;

		const RenderTexture m_titlePanelBaseTexture;
		const Vec2 m_titlePanelPosition;

		const Texture m_detailPanelBaseTexture;
		const Vec2 m_detailPanelPosition;

		const TiledTexture m_difficultyTexture;
		const TextureRegion m_difficultyTextureRegion;

		const int32 m_level;

		const NumberTextureFont m_numberTextureFont;
		const TextureFontTextLayout m_levelNumberLayout;
		const TextureFontTextLayout m_bpmNumberLayout;

		const HispeedSettingPanel m_hispeedSettingPanel;

	public:
		explicit SongInfoPanel(const kson::ChartData& chartData, FilePathView parentPath);

		void draw(double currentBPM, const Scroll::HighwayScrollContext& highwayScrollContext) const;
	};
}
