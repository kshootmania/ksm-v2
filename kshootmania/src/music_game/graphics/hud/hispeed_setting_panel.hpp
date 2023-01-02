#pragma once
#include "graphics/number_font_texture.hpp"
#include "music_game/hispeed_setting.hpp"
#include "music_game/graphics/highway/highway_scroll.hpp"

namespace MusicGame::Graphics
{
	class HispeedSettingPanel
	{
	private:
		const NumberFontTexture m_targetValueNumberFontTexture;
		const NumberFontTexture m_currentValueNumberFontTexture;

	public:
		HispeedSettingPanel();

		void draw(const Vec2& position, const HighwayScroll& highwayScroll) const;
	};
}
