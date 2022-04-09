#include "highway_3d_graphics.hpp"
#include "music_game_assets.hpp"

namespace
{
	// Angle of the line connecting the camera position and the judgment line from the horizontal
	// (Unconfirmed, but KSMv1 uses this value anyway)
	constexpr double kCameraToJudgmentLineRadians = -0.6125;

	constexpr Size kTextureSize = { 256, 1024 };
	constexpr double kJudgmentLineYFromBottom = 14;

	constexpr double kPlaneWidth = 304.0 * 2 / 13;
	constexpr double kPlaneHeight = 936.0 * 13 / 20;
	constexpr double kPlaneHeightBelowJudgmentLine = kPlaneHeight * kJudgmentLineYFromBottom / kTextureSize.y;
	constexpr double kPlaneHeightAboveJudgmentLine = kPlaneHeight - kPlaneHeightBelowJudgmentLine;

	// Shrink UV to remove white edge pixels
	constexpr float kUVShrinkX = 0.015f;
	constexpr float kUVShrinkY = 0.005f;

	constexpr double kCameraVerticalFOV = 45_deg;
	constexpr Vec3 kCameraPosition = { 0.0, 45.0, -366.0 };

	constexpr double kSin15Deg = 0.2588190451;
	constexpr double kCos15Deg = 0.9659258263;
	constexpr Vec3 kCameraLookAt = kCameraPosition + Vec3{ 0.0, -100.0 * kSin15Deg, 100.0 * kCos15Deg };
}

MusicGame::Highway3DGraphics::Highway3DGraphics()
	: m_bgTexture(U"imgs/" + MusicGameTexture::kHighwayBG, TextureDesc::UnmippedSRGB)
	, m_additiveRenderTexture(kTextureSize, TextureFormat::R8G8B8A8_Unorm_SRGB)
	, m_subtractiveRenderTexture(kTextureSize, TextureFormat::R8G8B8A8_Unorm_SRGB)
	, m_meshData(MeshData::Grid({ 0.0, 0.0, 0.0 }, { kPlaneWidth, kPlaneHeight }, 2, 2, { 1.0f - kUVShrinkX, 1.0f - kUVShrinkY }, { kUVShrinkX / 2, kUVShrinkY / 2 }))
	, m_mesh(m_meshData) // <- this initialization is needed because DynamicMesh::fill() doesn't change the size of the vertex array dynamically
	, m_camera(Scene::Size(), kCameraVerticalFOV, kCameraPosition, kCameraLookAt)
	, m_3dViewTexture(Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes)
{
}

void MusicGame::Highway3DGraphics::update(const CameraState& cameraState)
{
	// TODO: Calculate vertex position from cameraState
}

void MusicGame::Highway3DGraphics::draw() const
{
	Graphics3D::SetCameraTransform(m_camera);
	Graphics3D::SetGlobalAmbientColor(Palette::White);

	Shader::Copy(m_bgTexture(0, 0, kTextureSize), m_additiveRenderTexture);
	Shader::Copy(m_bgTexture(kTextureSize.x, 0, kTextureSize), m_subtractiveRenderTexture);

	// TODO: draw notes here

	const ColorF backgroundColor = Palette::Blue.removeSRGBCurve();
	{
		const ScopedRenderTarget3D renderTarget(m_3dViewTexture.clear(backgroundColor));

		{
			const ScopedRenderStates3D renderState(BlendState::Subtractive);
			m_mesh.draw(m_subtractiveRenderTexture);
		}

		{
			const ScopedRenderStates3D renderState(BlendState::Additive);
			m_mesh.draw(m_additiveRenderTexture);
		}
	}
	
	// Draw 3D scene to 2D scene
	{
		Graphics3D::Flush();
		m_3dViewTexture.resolve();
		Shader::LinearToScreen(m_3dViewTexture);
	}
}
