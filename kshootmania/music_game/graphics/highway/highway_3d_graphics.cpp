#include "highway_3d_graphics.hpp"
#include "music_game/graphics/graphics_defines.hpp"
#include "note/note_graphics_utils.hpp"

namespace
{
	using namespace MusicGame::Graphics;

	constexpr StringView kHighwayBGTextureFilename = U"base.gif";
	constexpr StringView kShineEffectTextureFilename = U"lanelight.gif";

	// Angle of the line connecting the camera position and the judgment line from the horizontal
	// (Unconfirmed, but KSMv1 uses this value anyway)
	constexpr double kCameraToJdglineRadians = -0.6125;

	constexpr double kJdglineYFromBottom = 14;

	constexpr double kPlaneHeightBelowJdgline = kHighwayPlaneSize.y * kJdglineYFromBottom / kHighwayTextureSize.y;
	constexpr double kPlaneHeightAboveJdgline = kHighwayPlaneSize.y - kPlaneHeightBelowJdgline;

	// Shrink UV to remove edge pixels
	constexpr float kUVShrinkX = 0.0075f;
	constexpr float kUVShrinkY = 0.005f;

	constexpr int32 kNumShineEffects = 4;
	constexpr Vec2 kShineEffectPositionOffset = { 40.0, 0.0 };
	constexpr Vec2 kShineEffectPositionDiff = { 0.0, 300.0 };
}

MusicGame::Graphics::Highway3DGraphics::Highway3DGraphics()
	: m_bgTexture(TextureAsset(kHighwayBGTextureFilename))
	, m_shineEffectTexture(TextureAsset(kShineEffectTextureFilename))
	, m_additiveRenderTexture(kHighwayTextureSize)
	, m_invMultiplyRenderTexture(kHighwayTextureSize)
	, m_meshData(MeshData::Grid({ 0.0, 0.0, 0.0 }, kHighwayPlaneSize, 2, 2, { 1.0f - kUVShrinkX, 1.0f - kUVShrinkY }, { kUVShrinkX / 2, kUVShrinkY / 2 }))
	, m_mesh(m_meshData) // <- this initialization is important because DynamicMesh::fill() does not dynamically resize the vertex array
{
}

void MusicGame::Graphics::Highway3DGraphics::draw(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget, double tiltRadians) const
{
	assert(updateInfo.pChartData != nullptr);

	const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);
	Shader::Copy(m_bgTexture(0, 0, kHighwayTextureSize), m_additiveRenderTexture);
	Shader::Copy(m_bgTexture(kHighwayTextureSize.x, 0, kHighwayTextureSize), m_invMultiplyRenderTexture);

	// Draw shine effect
	{
		const ScopedRenderTarget2D renderTarget(m_additiveRenderTexture);
		const ScopedRenderStates2D renderState(BlendState::Additive);

		for (int32 i = 0; i < kNumShineEffects; ++i)
		{
			m_shineEffectTexture.draw(kShineEffectPositionOffset + kShineEffectPositionDiff * i + kShineEffectPositionDiff * MathUtils::WrappedFmod(updateInfo.currentTimeSec, 0.2) / 0.2);
		}
	}

	// Draw BT/FX notes
	m_fxNoteGraphics.draw(updateInfo, m_additiveRenderTexture, m_invMultiplyRenderTexture);
	m_btNoteGraphics.draw(updateInfo, m_additiveRenderTexture, m_invMultiplyRenderTexture);

	// Draw key beams
	m_keyBeamGraphics.draw(updateInfo, m_additiveRenderTexture);

	// Draw laser notes
	m_laserNoteGraphics.draw(updateInfo, m_additiveRenderTexture, m_invMultiplyRenderTexture);

	// Draw highway into 3D space
	{
		const Mat4x4 m = Mat4x4::Rotate(Float3::Forward(), -tiltRadians, Float3{ 0.0f, 42.0f, 0.0f });
		const Transformer3D transform{ m };

		{
			const ScopedRenderTarget3D renderTarget(invMultiplyTarget);
			m_mesh.draw(m_invMultiplyRenderTexture);
		}

		{
			const ScopedRenderTarget3D renderTarget(additiveTarget);
			m_mesh.draw(m_additiveRenderTexture);
		}
	}
}
