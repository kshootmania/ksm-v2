#include "hispeed_setting_panel.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kNumberFontTextureFilename = U"num2.png";
		constexpr StringView kFPSTextureFilename = U"fps.png";
	}

	HispeedSettingPanel::HispeedSettingPanel()
		: m_targetValueNumberFontTexture(kNumberFontTextureFilename, ScreenUtils::Scaled(10, 9), { 20, 18 }, { 0, 0 })
		, m_currentValueNumberFontTexture(kNumberFontTextureFilename, ScreenUtils::Scaled(10, 9), { 20, 18 }, { 20, 0 })
	{
	}

	void HispeedSettingPanel::draw(const Vec2& position, const Scroll::HighwayScrollContext& highwayScrollContext) const
	{
		using namespace ScreenUtils;
		const ScopedRenderStates2D samplerState(SamplerState::ClampLinear);

		const Scroll::HighwayScroll& highwayScroll = highwayScrollContext.highwayScroll();
		const HispeedSetting& hispeedSetting = highwayScroll.hispeedSetting();
		m_targetValueNumberFontTexture.draw(position - Scaled(29, 0), hispeedSetting.value, 0, ZeroPaddingYN::No, NumberFontTexture::Align::Center);
		m_currentValueNumberFontTexture.draw(position + Scaled(29, 0), highwayScroll.currentHispeed(), 0, ZeroPaddingYN::No, NumberFontTexture::Align::Center);
	}
}
