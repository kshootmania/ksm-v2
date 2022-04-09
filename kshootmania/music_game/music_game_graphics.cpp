#include "music_game_graphics.hpp"

namespace
{
	constexpr double kCameraVerticalFOV = 45_deg;
	constexpr Vec3 kCameraPosition = { 0.0, 45.0, -366.0 };

	constexpr double kSin15Deg = 0.2588190451;
	constexpr double kCos15Deg = 0.9659258263;
	constexpr Vec3 kCameraLookAt = kCameraPosition + Vec3{ 0.0, -100.0 * kSin15Deg, 100.0 * kCos15Deg };
}

MusicGame::MusicGameGraphics::MusicGameGraphics()
	: m_camera(Scene::Size(), kCameraVerticalFOV, kCameraPosition, kCameraLookAt)
	, m_3dViewTexture(Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes)
{
}

void MusicGame::MusicGameGraphics::update(const CameraState& cameraState)
{
	m_highway3DGraphics.update(cameraState);
}

void MusicGame::MusicGameGraphics::draw() const
{
	Graphics3D::SetCameraTransform(m_camera);
	Graphics3D::SetGlobalAmbientColor(Palette::White);

	m_highway3DGraphics.draw(m_3dViewTexture);

	// Draw 3D scene to 2D scene
	{
		Graphics3D::Flush();
		m_3dViewTexture.resolve();
		Shader::LinearToScreen(m_3dViewTexture);
	}
}
