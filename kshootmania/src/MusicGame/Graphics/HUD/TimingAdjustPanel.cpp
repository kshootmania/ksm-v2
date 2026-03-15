#include "TimingAdjustPanel.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kLabelTextureFilename = U"timingadjust.png";
		constexpr StringView kSignTextureFilename = U"timingadjust_sign.png";
		constexpr StringView kMsTextureFilename = U"timingadjust_ms.png";
		constexpr StringView kNumberTextureFilename = U"num2.png";
	}

	TimingAdjustPanel::TimingAdjustPanel()
		: m_labelTexture(TextureAsset(kLabelTextureFilename))
		, m_signTexture(TextureAsset(kSignTextureFilename))
		, m_msTexture(TextureAsset(kMsTextureFilename))
		, m_numberFont(kNumberTextureFilename, { 20, 18 })
		, m_numberLayout(Scaled(10, 9), TextureFontTextLayout::Align::Left)
	{
	}

	void TimingAdjustPanel::draw(int32 timingAdjustMs) const
	{
		if (timingAdjustMs == 0)
		{
			return;
		}

		const ScopedRenderStates2D samplerState(SamplerState::ClampLinear);

		m_labelTexture.resized(Scaled(60, 20)).draw(Scaled(4, 436));

		const int32 signFrame = timingAdjustMs > 0 ? 1 : 0;
		m_signTexture(0, signFrame * 18, 18, 18).resized(Scaled(9, 9)).draw(Scaled(4, 460));

		m_numberFont.draw(m_numberLayout, Scaled(15, 460), Abs(timingAdjustMs), ZeroPaddingYN::No);

		const int32 digitCount = static_cast<int32>(Format(Abs(timingAdjustMs)).length());
		m_msTexture.resized(Scaled(30, 9)).draw(Scaled(17 + digitCount * 10, 460));
	}
}
