#pragma once
#include "graphics/number_font_texture.hpp"

namespace MusicGame::Graphics
{
	class FrameRateMonitor
	{
	private:
		const NumberFontTexture m_numberFontTexture;
		const Texture m_fpsTexture;

	public:
		FrameRateMonitor();

		void draw() const;
	};
}
