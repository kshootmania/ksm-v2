#pragma once
#include "music_game/game_defines.hpp"
#include "graphics/number_font_texture.hpp"

namespace MusicGame::Graphics
{
	class GaugePanel
	{
	private:
		const GaugeType m_gaugeType;
		const kson::Pulse m_intervalPulse;
		const TiledTexture m_baseTexture;
		const TiledTexture m_barTexture;
		const TiledTexture m_barAnimTexture;
		const Texture m_percentBaseTexture;
		const NumberFontTexture m_percentNumberFontTexture;

	public:
		explicit GaugePanel(GaugeType gaugeType);

		void draw(double percent, kson::Pulse currentPulse) const;
	};
}
