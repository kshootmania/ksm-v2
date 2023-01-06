#pragma once
#include "graphics/number_texture_font.hpp"

namespace MusicGame::Graphics
{
	class ScorePanel
	{
	private:
		const Texture m_captionTexture;
		const NumberTextureFont m_numberTextureFont;
		const TextureFontTextLayout m_numberLayout;

	public:
		ScorePanel();

		void draw(int32 score) const;
	};
}
