#pragma once
#include "graphics/number_texture_font.hpp"
#include "music_game/game_status.hpp"

namespace MusicGame::Graphics
{
	class ComboOverlay
	{
	private:
		const NumberTextureFont m_numberTextureFont;
		const NumberTextureFont m_numberTextureFontNoError;
		const TextureFontTextLayout m_numberLayout;
		const Texture m_comboTexture;
		const TextureRegion m_comboTextureRegion;
		const TextureRegion m_comboTextureRegionNoError;

		// 現在のコンボ数
		int32 m_combo = 0;

		// 表示中かどうかのタイマー
		Timer m_visibleTimer;

	public:
		ComboOverlay();

		void update(int32 combo);

		void draw() const;
	};
}
