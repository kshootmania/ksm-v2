#pragma once
#include "graphics/number_texture_font.hpp"
#include "music_game/scroll/highway_scroll.hpp"

namespace MusicGame::Graphics
{
	class HispeedSettingPanel
	{
	private:
		const NumberTextureFont m_targetValueNumberTextureFont;
		const NumberTextureFont m_currentValueNumberTextureFont;
		const TiledTexture m_hispeedLetterTexture;
		const TextureFontTextLayout m_numberLayout;

	public:
		HispeedSettingPanel();

		void draw(const Vec2& position, const Scroll::HighwayScrollContext& highwayScrollContext) const;
	};
}
