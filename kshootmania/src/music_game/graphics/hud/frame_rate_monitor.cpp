#include "frame_rate_monitor.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kNumberTextureFontFilename = U"num2.png";
		constexpr StringView kFPSTextureFilename = U"fps.png";
	}

	FrameRateMonitor::FrameRateMonitor()
		: m_numberTextureFont(kNumberTextureFontFilename, { 20, 18 })
		, m_numberLayout(Scaled(10, 9), TextureFontTextLayout::Align::Right)
		, m_fpsTexture(TextureAsset(kFPSTextureFilename))
	{
	}

	void FrameRateMonitor::draw() const
	{
		const ScopedRenderStates2D samplerState(SamplerState::ClampLinear);

		m_fpsTexture.resized(Scaled(30, 9)).draw(Scene::Width() - Scaled(38), Scaled(460));
		m_numberTextureFont.draw(m_numberLayout, { Scene::Width() - Scaled(40), Scaled(460) }, Profiler::FPS(), ZeroPaddingYN::No);
	}
}
