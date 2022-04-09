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
}

MusicGame::Highway3DGraphics::Highway3DGraphics()
	: m_bgTexture(U"imgs/" + MusicGameTexture::kHighwayBG, TextureDesc::UnmippedSRGB)
	, m_3dViewTexture(Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes)
	, m_additiveMeshData(MeshData::Grid({ 0.0, 0.0, 0.0 }, { kPlaneWidth, kPlaneHeight }, 2, 2, { (1.0f - kUVShrinkX) / 2, 1.0f - kUVShrinkY }, { (kUVShrinkX / 2) / 2, kUVShrinkY / 2 }))
	, m_additiveMesh(m_additiveMeshData) // <- this initialization is needed because DynamicMesh::fill() doesn't change the size of the vertex array dynamically
	, m_subtractiveMeshData(MeshData::Grid({ 0.0, 0.0, 0.0 }, { kPlaneWidth, kPlaneHeight }, 2, 2, { (1.0f - kUVShrinkX) / 2, 1.0f - kUVShrinkY }, { (1.0f + kUVShrinkX / 2) / 2, kUVShrinkY / 2 }))
	, m_subtractiveMesh(m_subtractiveMeshData) // <- this initialization is needed because DynamicMesh::fill() doesn't change the size of the vertex array dynamically
	, m_debugCamera(m_3dViewTexture.size(), 45_deg, { 0.0, 45.0, -366.0 }, { 0.0, 45.0 - 100.0 * Sin(ToRadians(15)), -366.0 + 100.0 * Cos(ToRadians(15)) })
{
}

void MusicGame::Highway3DGraphics::update(const CameraState& cameraState)
{
	//m_debugCamera.update(2.0);

	// TODO: Calculate vertex position from cameraState
}

void MusicGame::Highway3DGraphics::draw() const
{
	Graphics3D::SetCameraTransform(m_debugCamera);
	Graphics3D::SetGlobalAmbientColor(Palette::White);

	const ColorF backgroundColor = Palette::Blue.removeSRGBCurve();
	{
		const ScopedRenderTarget3D renderTarget(m_3dViewTexture.clear(backgroundColor));

		{
			const ScopedRenderStates3D renderState(BlendState::Subtractive);
			m_subtractiveMesh.draw(m_bgTexture);
		}

		{
			const ScopedRenderStates3D renderState(BlendState::Additive);
			m_additiveMesh.draw(m_bgTexture);
		}
	}
	
	// Draw 3D scene to 2D scene
	{
		Graphics3D::Flush();
		m_3dViewTexture.resolve();
		Shader::LinearToScreen(m_3dViewTexture);
	}
}
