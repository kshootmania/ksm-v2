#include "gauge_panel.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kBaseTextureFilename = U"er.png";
		constexpr StringView kBarTextureFilename = U"er_g.gif";
		constexpr StringView kBarAnimTextureFilename = U"er_g_pattern.gif";
		constexpr StringView kPercentBaseTextureFilename = U"er_p.png";
		constexpr StringView kPercentNumberTextureFontFilename = U"num2.png";

		constexpr SizeF kBarSize = { 47.25, 434.0 };
		constexpr SizeF kBarAnimSize = { kBarSize.x, kBarSize.y * 2 };

		double AnimRate(kson::Pulse currentPulse)
		{
			constexpr kson::Pulse kIntervalPulse = kson::kResolution4 * 3 / 2; // 1.5小節
			return static_cast<double>(MathUtils::WrappedMod(currentPulse, kIntervalPulse)) / kIntervalPulse;
		}
	}

	GaugePanel::GaugePanel(GaugeType gaugeType)
		: m_gaugeType(gaugeType)
		, m_baseTexture(kBaseTextureFilename,
			{
				.column = kNumGaugeTypes * 2,
				.sourceScale = ScreenUtils::SourceScale::k2x,
				.sourceSize = { 192, 570 },
			})
		, m_barTexture(kBarTextureFilename,
			{
				.column = kNumGaugeTypes * 2,
				.sourceScale = ScreenUtils::SourceScale::k2x,
				.sourceSize = { 48, 434 },
			})
		, m_barAnimTexture(kBarAnimTextureFilename,
			{
				.column = kNumGaugeTypes * 2,
				.sourceScale = ScreenUtils::SourceScale::k2x,
				.sourceSize = { 48, 868 },
			})
		, m_percentBaseTexture(TextureAsset(kPercentBaseTextureFilename))
		, m_percentNumberTextureFont(kPercentNumberTextureFontFilename, { 20, 18 })
		, m_percentNumberLayout(ScreenUtils::Scaled2x(20, 18), TextureFontTextLayout::Align::Right)
	{
	}

	void GaugePanel::draw(double percent, kson::Pulse currentPulse) const
	{
		using namespace ScreenUtils;
		const ScopedRenderStates2D samplerState(SamplerState::ClampAniso);

		const double percentThreshold = (m_gaugeType == kHardGauge) ? 30.0 : 70.0;
		const int32 column = m_gaugeType * 2 + ((percent < percentThreshold) ? 0 : 1);

		const Vec2 basePosition = { Scene::Width() / 2 + Scaled(155), Scaled(88) };
		m_baseTexture(0, column).draw(basePosition);

		{
			const ScopedRenderStates2D renderState(BlendState::Additive);

			const double barHeight = kBarSize.y * percent / 100;
			const Vec2 barBasePosition = basePosition + Scaled2x(34, 126);
			const RectF barClipRect = { barBasePosition + Vec2::Down(Scaled2x(kBarSize.y - barHeight)), Scaled2x(SizeF{ kBarSize.x, barHeight }) };
			m_barTexture(0, column).resized(Scaled2x(kBarSize)).drawClipped(barBasePosition, barClipRect);

			const double animRate = AnimRate(currentPulse);
			m_barAnimTexture(0, column).resized(Scaled2x(kBarAnimSize)).drawClipped(barBasePosition + Vec2::Up(Scaled2x(kBarAnimSize.y) * animRate), barClipRect);
			m_barAnimTexture(0, column).resized(Scaled2x(kBarAnimSize)).drawClipped(barBasePosition + Vec2::Up(Scaled2x(kBarAnimSize.y) * (animRate - 1.0)), barClipRect);
		}

		const Vec2 percentBasePosition = basePosition + Scaled2x(Vec2{ 72, 106 + 431 * (1.0 - percent / 100) });
		Scaled2x(m_percentBaseTexture).draw(percentBasePosition);

		const Vec2 percentNumberBasePosition = percentBasePosition + Scaled2x(Vec2{ 72, 13 });
		m_percentNumberTextureFont.draw(m_percentNumberLayout, percentNumberBasePosition, static_cast<int>(percent), ZeroPaddingYN::No);
	}
}
