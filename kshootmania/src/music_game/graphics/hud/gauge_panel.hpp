#pragma once
#include "music_game/game_defines.hpp"
#include "graphics/number_texture_font.hpp"

namespace MusicGame::Graphics
{
	class GaugePanel
	{
	private:
		const GaugeType m_gaugeType;
		const TiledTexture m_baseTexture;
		const TiledTexture m_barTexture;
		const TiledTexture m_barAnimTexture;
		const Texture m_percentBaseTexture;
		const NumberTextureFont m_percentNumberTextureFont;
		const TextureFontTextLayout m_percentNumberLayout;

	public:
		explicit GaugePanel(GaugeType gaugeType);

		void draw(double percent, kson::Pulse currentPulse) const;
	};
}
