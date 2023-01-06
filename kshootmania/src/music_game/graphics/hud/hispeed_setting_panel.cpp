#include "hispeed_setting_panel.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kNumberTextureFontFilename = U"num2.png";
		constexpr StringView kFPSTextureFilename = U"fps.png";
	}

	HispeedSettingPanel::HispeedSettingPanel()
		: m_targetValueNumberTextureFont(kNumberTextureFontFilename, { 20, 18 }, { 0, 0 })
		, m_currentValueNumberTextureFont(kNumberTextureFontFilename, { 20, 18 }, { 20, 0 })
		, m_numberLayout(ScreenUtils::Scaled(10, 9), TextureFontTextLayout::Align::Center)
	{
	}

	void HispeedSettingPanel::draw(const Vec2& position, const Scroll::HighwayScrollContext& highwayScrollContext) const
	{
		using namespace ScreenUtils;
		const ScopedRenderStates2D samplerState(SamplerState::ClampLinear);

		const Scroll::HighwayScroll& highwayScroll = highwayScrollContext.highwayScroll();
		const HispeedSetting& hispeedSetting = highwayScroll.hispeedSetting();
		m_targetValueNumberTextureFont.draw(m_numberLayout, position - Scaled(29, 0), hispeedSetting.value, ZeroPaddingYN::No);
		m_currentValueNumberTextureFont.draw(m_numberLayout, position + Scaled(29, 0), highwayScroll.currentHispeed(), ZeroPaddingYN::No);
	}
}
