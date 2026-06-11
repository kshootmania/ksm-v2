#include "ComboOverlay.hpp"

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

		// 100コンボ毎のアニメーション
		constexpr int32 kMilestoneFrameCount = 4;
		constexpr Size kMilestoneTextureSourceSize = { 180, 60 };
		constexpr Point kMilestoneTextureSourceOffset = { 0, kNumberTextureSourceOffset.y + kNumberTextureSourceSize.y * 10 };
		constexpr double kMilestoneDurationSec = 0.3;
		constexpr double kMilestoneFadeInEndSec = 0.07;
		constexpr double kMilestoneFadeOutStartSec = 0.23;
		constexpr double kMilestoneFrameDurationSec = kMilestoneDurationSec / kMilestoneFrameCount;
	}

	ComboOverlay::ComboOverlay()
		: m_numberTextureFont(kTextureFilename, kNumberTextureSourceSize, kNumberTextureSourceOffset)
		, m_numberTextureFontNoError(kTextureFilename, kNumberTextureSourceSize, kNumberTextureSourceOffsetNoError)
		, m_numberLayout(Scaled(40, 50), TextureFontTextLayout::Align::Center, 4, Scaled(32))
		, m_comboTexture(TextureAsset(kTextureFilename))
		, m_comboTextureRegion(m_comboTexture(kComboTextureSourceOffset, kComboTextureSourceSize).resized(Scaled(80, 20)))
		, m_comboTextureRegionNoError(m_comboTexture(kComboTextureSourceOffsetNoError, kComboTextureSourceSize).resized(Scaled(80, 20)))
		, m_milestoneTexture(kTextureFilename, TiledTextureSizeInfo
			{
				.row = kMilestoneFrameCount,
				.sourceOffset = kMilestoneTextureSourceOffset,
				.sourceSize = kMilestoneTextureSourceSize,
			})
		, m_visibleTimer(kVisibleDuration)
	{
	}

	void ComboOverlay::update(const ViewStatus& viewStatus)
	{
		// TODO: 楽曲時間を使うべき？
		if (viewStatus.displayCombo == 0)
		{
			m_visibleTimer.reset(); // コンボが切れた時は非表示にする
		}
		else if (m_combo != viewStatus.displayCombo && !m_isFirstUpdate)
		{
			m_visibleTimer.restart(); // コンボ数増加から一定時間表示
		}

		m_combo = viewStatus.displayCombo;
		m_isNoError = viewStatus.displayIsNoError;
		m_milestoneEffectStartTimeSec = viewStatus.comboMilestoneEffectStartTimeSec;
		m_isFirstUpdate = false;
	}

	void ComboOverlay::draw(double currentTimeSec) const
	{
		const bool isVisible = m_visibleTimer.isRunning() && !m_visibleTimer.reachedZero();
		const double milestoneElapsedSec = currentTimeSec - m_milestoneEffectStartTimeSec;
		const bool isMilestoneVisible = 0.0 <= milestoneElapsedSec && milestoneElapsedSec < kMilestoneDurationSec;
		if (!isVisible && !isMilestoneVisible)
		{
			return;
		}

		const ScopedRenderStates2D blendState(BlendState::Additive);
		const ScopedRenderStates2D samplerState(SamplerState::ClampLinear);

		if (isVisible)
		{
			const double shakeX = Scaled(1.0) * (Periodic::Square0_1(0.05s) - 0.5); // TODO: 楽曲時間を使うべき？

			const TextureRegion& comboTextureRegion = m_isNoError ? m_comboTextureRegionNoError : m_comboTextureRegion;
			comboTextureRegion.drawAt(Scene::Width() / 2 + shakeX, Scaled(300));

			const NumberTextureFont& numberTextureFont = m_isNoError ? m_numberTextureFontNoError : m_numberTextureFont;
			numberTextureFont.draw(m_numberLayout, { Scene::Width() / 2 + shakeX, Scaled(313) }, m_combo, ZeroPaddingYN::Yes);
		}

		if (isMilestoneVisible)
		{
			// 開始からの経過秒に応じてフレーム送りとフェードを行う
			double alpha;
			if (milestoneElapsedSec < kMilestoneFadeInEndSec)
			{
				alpha = milestoneElapsedSec / kMilestoneFadeInEndSec;
			}
			else if (milestoneElapsedSec < kMilestoneFadeOutStartSec)
			{
				alpha = 1.0;
			}
			else
			{
				alpha = Max((kMilestoneDurationSec - milestoneElapsedSec) / (kMilestoneDurationSec - kMilestoneFadeOutStartSec), 0.0);
			}
			const int32 frame = Min(static_cast<int32>(milestoneElapsedSec / kMilestoneFrameDurationSec), kMilestoneFrameCount - 1);
			m_milestoneTexture(frame).resized(Scaled(120, 40)).drawAt(Scene::Width() / 2.0, Scaled(339), ColorF{ 1.0, alpha });
		}
	}
}
