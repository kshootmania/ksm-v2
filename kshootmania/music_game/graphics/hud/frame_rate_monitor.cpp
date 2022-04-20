#include "frame_rate_monitor.hpp"

namespace
{
	constexpr StringView kNumberFontTextureFilename = U"num2.png";
	constexpr StringView kFPSTextureFilename = U"fps.png";
}

MusicGame::Graphics::FrameRateMonitor::FrameRateMonitor()
	: m_numberFontTexture(kNumberFontTextureFilename, ScreenUtils::Scaled(10, 9), { 20, 18 })
	, m_fpsTexture(TextureAsset(kFPSTextureFilename))
{
}

void MusicGame::Graphics::FrameRateMonitor::draw() const
{
	using namespace ScreenUtils;

	m_fpsTexture.resized(Scaled(30, 9)).draw(Scene::Width() - Scaled(38), Scaled(460));
	m_numberFontTexture.draw({ Scene::Width() - Scaled(40), Scaled(460) }, Profiler::FPS(), 0, false);
}
