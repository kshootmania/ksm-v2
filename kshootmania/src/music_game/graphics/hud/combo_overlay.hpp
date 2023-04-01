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

		// 直近の更新時のComboStatusの値
		ComboStatus m_comboStatus;

		// 表示中かどうかのタイマー
		Timer m_visibleTimer;

	public:
		ComboOverlay();

		void update(const ComboStatus& comboStatus);

		void draw() const;
	};
}
