#include "highway_3d_graphics.hpp"
#include "music_game/graphics/graphics_defines.hpp"
#include "note/note_graphics_utils.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kHighwayBaseTextureFilename = U"base.gif";
		constexpr StringView kShineEffectTextureFilename = U"lanelight.gif";

		// カメラ座標と判定ラインを線で結んだ場合の垂直からの角度
		// (値の根拠は不明だが、KSMv1でこの値が使用されていたためそのまま持ってきている)
		constexpr double kCameraToJdglineRadians = -0.6125;

		constexpr double kJdglineYFromBottom = 14;

		constexpr double kPlaneHeightBelowJdgline = kHighwayPlaneSize.y * kJdglineYFromBottom / kHighwayTextureSize.y;
		constexpr double kPlaneHeightAboveJdgline = kHighwayPlaneSize.y - kPlaneHeightBelowJdgline;

		// UV座標の縮め幅(端にテクスチャの折り返しピクセルが見える現象の対策)
		constexpr float kUVShrinkX = 0.0075f;
		constexpr float kUVShrinkY = 0.005f;

		constexpr int32 kNumShineEffects = 4;
		constexpr Vec2 kShineEffectPositionOffset = { 40.0, 0.0 };
		constexpr Vec2 kShineEffectPositionDiff = { 0.0, 300.0 };
	}

	Highway3DGraphics::Highway3DGraphics()
		: m_baseTexture(TextureAsset(kHighwayBaseTextureFilename))
		, m_shineEffectTexture(TextureAsset(kShineEffectTextureFilename))
		, m_additiveRenderTexture(kHighwayTextureSize)
		, m_invMultiplyRenderTexture(kHighwayTextureSize)
		, m_meshData(MeshData::Grid({ 0.0, 0.0, 0.0 }, kHighwayPlaneSize, 2, 2, { 1.0f - kUVShrinkX, 1.0f - kUVShrinkY }, { kUVShrinkX / 2, kUVShrinkY / 2 }))
		, m_mesh(m_meshData) // DynamicMesh::fill()で頂点データの配列サイズが動的に変更される訳ではないのでこの初期化は必須
	{
	}

	void Highway3DGraphics::draw2D(const kson::ChartData& chartData, const GameStatus& gameStatus) const
	{
		const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);
		Shader::Copy(m_baseTexture(0, 0, kHighwayTextureSize), m_additiveRenderTexture);
		Shader::Copy(m_baseTexture(kHighwayTextureSize.x, 0, kHighwayTextureSize), m_invMultiplyRenderTexture);

		// Draw shine effect
		{
			const ScopedRenderTarget2D renderTarget(m_additiveRenderTexture);
			const ScopedRenderStates2D renderState(BlendState::Additive);

			for (int32 i = 0; i < kNumShineEffects; ++i)
			{
				m_shineEffectTexture.draw(kShineEffectPositionOffset + kShineEffectPositionDiff * i + kShineEffectPositionDiff * MathUtils::WrappedFmod(gameStatus.currentTimeSec, 0.2) / 0.2);
			}
		}

		// BT/FXノーツの描画
		m_buttonNoteGraphics.draw(chartData, gameStatus, m_additiveRenderTexture, m_invMultiplyRenderTexture);

		// キービームの描画
		m_keyBeamGraphics.draw(gameStatus, m_additiveRenderTexture);

		// レーザーノーツの描画
		m_laserNoteGraphics.draw(chartData, gameStatus, m_additiveRenderTexture, m_invMultiplyRenderTexture);
	}

	void Highway3DGraphics::draw3D(double tiltRadians) const
	{
		// レンダーテクスチャを3D空間上へ描画
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
}
