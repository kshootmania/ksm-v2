#include "highway_3d_graphics.hpp"
#include "music_game/graphics/graphics_defines.hpp"
#include "note/note_graphics_utils.hpp"

namespace
{
	using namespace MusicGame::Graphics;

	constexpr StringView kHighwayBaseTextureFilename = U"base.gif";
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
	: m_baseTexture(TextureAsset(kHighwayBaseTextureFilename))
	, m_shineEffectTexture(TextureAsset(kShineEffectTextureFilename))
	, m_additiveRenderTexture(kHighwayTextureSize)
	, m_invMultiplyRenderTexture(kHighwayTextureSize)
	, m_meshData(MeshData::Grid({ 0.0, 0.0, 0.0 }, kHighwayPlaneSize, 2, 2, { 1.0f - kUVShrinkX, 1.0f - kUVShrinkY }, { kUVShrinkX / 2, kUVShrinkY / 2 }))
	, m_mesh(m_meshData) // <- this initialization is required because DynamicMesh::fill() does not resize the vertex array dynamically
{
}

void MusicGame::Graphics::Highway3DGraphics::draw2D(const UpdateInfo& updateInfo) const
{
	assert(updateInfo.pChartData != nullptr);

	const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);
	Shader::Copy(m_baseTexture(0, 0, kHighwayTextureSize), m_additiveRenderTexture);
	Shader::Copy(m_baseTexture(kHighwayTextureSize.x, 0, kHighwayTextureSize), m_invMultiplyRenderTexture);

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
	m_buttonNoteGraphics.draw(updateInfo, m_additiveRenderTexture, m_invMultiplyRenderTexture);

	// Draw key beams
	m_keyBeamGraphics.draw(updateInfo, m_additiveRenderTexture);

	// Draw laser notes
	m_laserNoteGraphics.draw(updateInfo, m_additiveRenderTexture, m_invMultiplyRenderTexture);
}

void MusicGame::Graphics::Highway3DGraphics::draw3D(double tiltRadians) const
{
	// Draw 2D render texture into 3D plane
	const Transformer3D transform(TiltTransformMatrix(tiltRadians));

	{
		const ScopedRenderStates3D renderState(kInvMultiply);
		m_mesh.draw(m_invMultiplyRenderTexture);
	}

	{
		const ScopedRenderStates3D renderState(BlendState::Additive);
		m_mesh.draw(m_additiveRenderTexture);
	}
}
