#pragma once
#include "Graphics/NumberTextureFont.hpp"
#include "MusicGame/ViewStatus.hpp"

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

		// 100コンボ毎のアニメーションのテクスチャ
		const TiledTexture m_milestoneTexture;

		// TODO: Graphics内部でステートを持たないようにする(エディタでプレビューできなくなるので)

		// 直近更新時のコンボ数
		// (0コンボが表示されないよう初期値は0にする)
		int32 m_combo = 0;

		// 直近更新時にエラーが1つも出ていない状態かどうか
		bool m_isNoError = true;

		// 初回更新かどうか
		bool m_isFirstUpdate = true;

		// 表示中かどうかのタイマー
		Timer m_visibleTimer;

		// 100コンボ毎のアニメーションの開始時刻(秒)
		double m_milestoneEffectStartTimeSec = ViewStatus::kPastDisplayTimeSec;

	public:
		ComboOverlay();

		void update(const ViewStatus& viewStatus);

		void draw(double currentTimeSec) const;
	};
}
