#include "combo_overlay.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kTextureFilename = U"combonum.gif";

		constexpr Size kComboTextureSourceSize = { 120, 30 };
		constexpr Size kComboTextureSourceOffset = { 0, 0 };
		constexpr Size kComboTextureSourceOffsetNoError = { 0, kComboTextureSourceSize.y };

		constexpr Size kNumberTextureSourceSize = { 64, 75 };
		constexpr Size kNumberTextureSourceOffset = { 0, kComboTextureSourceSize.y * 2 };
		constexpr Size kNumberTextureSourceOffsetNoError = { kNumberTextureSourceSize.x, kNumberTextureSourceOffset.y };

		constexpr Duration kVisibleDuration = 0.5s;
	}

	ComboOverlay::ComboOverlay()
		: m_numberTextureFont(kTextureFilename, kNumberTextureSourceSize, kNumberTextureSourceOffset)
		, m_numberTextureFontNoError(kTextureFilename, kNumberTextureSourceSize, kNumberTextureSourceOffsetNoError)
		, m_numberLayout(ScreenUtils::Scaled(40, 50), TextureFontTextLayout::Align::Center, 4, ScreenUtils::Scaled(32))
		, m_comboTexture(TextureAsset(kTextureFilename))
		, m_comboTextureRegion(m_comboTexture(kComboTextureSourceOffset, kComboTextureSourceSize).resized(ScreenUtils::Scaled(80, 20)))
		, m_comboTextureRegionNoError(m_comboTexture(kComboTextureSourceOffsetNoError, kComboTextureSourceSize).resized(ScreenUtils::Scaled(80, 20)))
		, m_visibleTimer(kVisibleDuration)
	{
	}

	void ComboOverlay::update(const ViewStatus& viewStatus)
	{
		// TODO: 楽曲時間を使うべき？
		if (viewStatus.combo == 0)
		{
			m_visibleTimer.reset(); // コンボが切れた時は非表示にする
		}
		else if (m_combo != viewStatus.combo)
		{
			m_visibleTimer.restart(); // コンボ数増加から一定時間表示
		}

		m_combo = viewStatus.combo;
		m_isNoError = viewStatus.isNoError;
	}

	void ComboOverlay::draw() const
	{
		const bool isVisible = m_visibleTimer.isRunning() && !m_visibleTimer.reachedZero();
		if (!isVisible)
		{
			return;
		}

		using namespace ScreenUtils;
		const ScopedRenderStates2D blendState(BlendState::Additive);
		const ScopedRenderStates2D samplerState(SamplerState::ClampLinear);

		const double shakeX = Scaled(1.0) * (Periodic::Square0_1(0.05s) - 0.5); // TODO: 楽曲時間を使うべき？

		const TextureRegion& comboTextureRegion = m_isNoError ? m_comboTextureRegionNoError : m_comboTextureRegion;
		comboTextureRegion.drawAt(Scene::Width() / 2 + shakeX, Scaled(300));

		const NumberTextureFont& numberTextureFont = m_isNoError ? m_numberTextureFontNoError : m_numberTextureFont;
		numberTextureFont.draw(m_numberLayout, { Scene::Width() / 2 + shakeX, Scaled(313) }, m_combo, ZeroPaddingYN::Yes);
	}
}
