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
	, m_additiveRenderTexture(kTextureSize, TextureFormat::R8G8B8A8_Unorm_SRGB)
	, m_subtractiveRenderTexture(kTextureSize, TextureFormat::R8G8B8A8_Unorm_SRGB)
	, m_meshData(MeshData::Grid({ 0.0, 0.0, 0.0 }, { kPlaneWidth, kPlaneHeight }, 2, 2, { 1.0f - kUVShrinkX, 1.0f - kUVShrinkY }, { kUVShrinkX / 2, kUVShrinkY / 2 }))
	, m_mesh(m_meshData) // <- this initialization is needed because DynamicMesh::fill() doesn't change the size of the vertex array dynamically
{
}

void MusicGame::Highway3DGraphics::update(const CameraState& cameraState)
{
	// TODO: Calculate vertex position from cameraState
}

void MusicGame::Highway3DGraphics::draw(const RenderTexture& target) const
{
	Shader::Copy(m_bgTexture(0, 0, kTextureSize), m_additiveRenderTexture);
	Shader::Copy(m_bgTexture(kTextureSize.x, 0, kTextureSize), m_subtractiveRenderTexture);

	// TODO: draw notes here

	{
		const ScopedRenderTarget3D renderTarget(target.clear(Palette::Black));

		{
			const ScopedRenderStates3D renderState(BlendState::Subtractive);
			m_mesh.draw(m_subtractiveRenderTexture);
		}

		{
			const ScopedRenderStates3D renderState(BlendState::Additive);
			m_mesh.draw(m_additiveRenderTexture);
		}
	}
}
