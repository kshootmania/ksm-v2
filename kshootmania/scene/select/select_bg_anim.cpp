#include "select_bg_anim.hpp"
#include "select_assets.hpp"

namespace
{
	constexpr double kPeriodSec = 4.5;
	constexpr double kScrollPeriodSec = 18.0;
	constexpr double kWidth = 853.0;
	constexpr double kHeight = 12.0;

	double AlphaValue(double sec)
	{
		constexpr double kMinValue = 64.0 / 256;
		constexpr double kMaxValue = 1.0;
		return kMaxValue + kMinValue - Min(Sin(Fmod(sec, kPeriodSec) * Math::Pi / kPeriodSec) * 2 + kMinValue, kMaxValue);
	}

	double XOffsetValue(double sec)
	{
		return kWidth * Fmod(sec, kScrollPeriodSec) / kScrollPeriodSec;
	}

	double HeightValue(double sec)
	{
		constexpr double kOffsetSec = kPeriodSec / 2;
		constexpr double kPeriodicFactor = 20.0;
		return Min(kPeriodicFactor * Sin(Fmod(sec + kOffsetSec, kPeriodSec) * Math::Pi / kPeriodSec), kHeight);
	}

	double YOffsetValue(double sec)
	{
		return kHeight / 2 - HeightValue(sec) / 2;
	}

	Vec2 PosValue(double sec)
	{
		return { 0.0, 11.0 + YOffsetValue(sec) };
	}

	RectF RectValue(double sec)
	{
		return { XOffsetValue(sec) * 2, YOffsetValue(sec) * 2, (kWidth - XOffsetValue(sec)) * 2, HeightValue(sec) * 2 };
	}
}

SelectBGAnim::SelectBGAnim()
	: m_texture(TextureAsset(SelectTexture::kBGAnim))
	, m_stopwatch(StartImmediately::Yes)
{
}

void SelectBGAnim::draw() const
{
	using namespace ScreenUtils;

	const ScopedRenderStates2D additive(BlendState::Additive);
	const double sec = m_stopwatch.sF();
	const double alpha = AlphaValue(sec);
	for (int i = 0; i < 2; ++i)
	{
		RectF rectValue = RectValue(sec);
		Vec2 posValue = PosValue(sec);
		if (i == 1)
		{
			posValue.y += kBaseScreenHeight - 50.0;
		}

		// ループ用に左右2つ描画
		m_texture(rectValue).resized(Scaled2x(rectValue.w), Scaled2x(rectValue.h)).draw(Scaled(posValue), ColorF{ 1.0, alpha });
		rectValue.w = rectValue.x;
		posValue.x += kWidth - rectValue.x / 2;
		rectValue.x = 0.0;
		m_texture(rectValue).resized(Scaled2x(rectValue.w), Scaled2x(rectValue.h)).draw(Scaled(posValue), ColorF{ 1.0, alpha });
	}
}
