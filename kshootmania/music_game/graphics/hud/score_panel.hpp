#pragma once
#include "graphics/number_font_texture.hpp"

namespace MusicGame::Graphics
{
	class ScorePanel
	{
	private:
		const Texture m_captionTexture;
		const NumberFontTexture m_numberFontTexture;

	public:
		ScorePanel();

		void draw(int32 score) const;
	};
}
