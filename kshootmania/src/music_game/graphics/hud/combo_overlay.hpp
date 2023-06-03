#pragma once
#include "graphics/number_texture_font.hpp"
#include "music_game/view_status.hpp"

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

		// TODO: Graphics内部でステートを持たないようにする(エディタでプレビューできなくなるので)

		// 直近更新時のコンボ数
		// (0コンボが表示されないよう初期値は0にする)
		int32 m_combo = 0;

		// 直近更新時にエラーが1つも出ていない状態かどうか
		bool m_isNoError = true;

		// 表示中かどうかのタイマー
		Timer m_visibleTimer;

	public:
		ComboOverlay();

		void update(const ViewStatus& viewStatus);

		void draw() const;
	};
}
