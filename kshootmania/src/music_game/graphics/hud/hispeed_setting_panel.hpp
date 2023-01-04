#pragma once
#include "graphics/number_font_texture.hpp"
#include "music_game/scroll/highway_scroll.hpp"

namespace MusicGame::Graphics
{
	class HispeedSettingPanel
	{
	private:
		const NumberFontTexture m_targetValueNumberFontTexture;
		const NumberFontTexture m_currentValueNumberFontTexture;

	public:
		HispeedSettingPanel();

		void draw(const Vec2& position, const Scroll::HighwayScrollContext& highwayScrollContext) const;
	};
}
