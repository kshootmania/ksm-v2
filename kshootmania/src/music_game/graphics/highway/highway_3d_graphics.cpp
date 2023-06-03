#include "highway_3d_graphics.hpp"
#include "music_game/graphics/graphics_defines.hpp"
#include "music_game/camera/camera_math.hpp"
#include "note/note_graphics_utils.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kShineEffectTextureFilename = U"lanelight.gif";

		// カメラ座標と判定ラインを線で結んだ場合の垂直からの角度
		// (値の根拠は不明だが、KSMv1でこの値が使用されていたためそのまま持ってきている)
		constexpr double kCameraToJdglineRadians = -0.6125;

		constexpr double kJdglineYFromBottom = 14;

		constexpr double kPlaneHeight = kHighwayPlaneSize.y;
		constexpr double kPlaneHeightBelowJdgline = kPlaneHeight * kJdglineYFromBottom / kHighwayTextureSize.y;
		constexpr double kPlaneHeightAboveJdgline = kPlaneHeight - kPlaneHeightBelowJdgline;

		// UV座標の縮め幅(端にテクスチャの折り返しピクセルが見える現象の対策)
		constexpr float kUVShrinkX = 0.0075f;
		constexpr float kUVShrinkY = 0.005f;

		constexpr int32 kNumShineEffects = 4;
		constexpr Vec2 kShineEffectPositionOffset = kLanePositionOffset - Vec2{ 4.0, 0.0 };
		constexpr Vec2 kShineEffectPositionDiff = { 0.0, 300.0 };
		constexpr double kShineEffectLoopSec = 0.2;

		constexpr double kHighwayRotationXByCamera = ToRadians(60.0);
	}

	Highway3DGraphics::Highway3DGraphics()
		: m_shineEffectTexture(TextureAsset(kShineEffectTextureFilename))
		, m_meshData(MeshData::Grid({ 0.0, 0.0, 0.0 }, kHighwayPlaneSizeWide, 1, 1, { 1.0f - kUVShrinkX, 1.0f - kUVShrinkY }, { kUVShrinkX / 2, kUVShrinkY / 2 }))
		, m_mesh(m_meshData) // DynamicMesh::fill()で頂点データの配列サイズが動的に変更される訳ではないのでこの初期化は必須
	{
	}

	void Highway3DGraphics::update(const ViewStatus& viewStatus)
	{
		// メッシュの頂点座標を更新
		// HSP版の該当箇所: https://github.com/m4saka/kshootmania-v1-hsp/blob/d2811a09e2d75dad5cc152d7c4073897061addb7/src/scene/play/play_draw_frame.hsp#L779-L821

		const auto& camStatus = viewStatus.camStatus;
		const double zoom = Camera::ScaledCamZoomValue(camStatus.zoom);
		const double rotationX = ToRadians(camStatus.rotationX * 360 / 2400);
		const double sinRotationX = Sin(rotationX);
		const double cosRotationX = Cos(rotationX);

		m_meshData.vertices[0].pos.y = kPlaneHeightAboveJdgline * sinRotationX / 2.5; // 奥の辺 上方向
		m_meshData.vertices[1].pos.y = m_meshData.vertices[0].pos.y;
		m_meshData.vertices[2].pos.y = -zoom * 100 * Sin(kCameraToJdglineRadians) * kPlaneHeight / kPlaneHeightAboveJdgline - kPlaneHeightBelowJdgline * sinRotationX / 2.5; // 手前の辺 上方向
		m_meshData.vertices[3].pos.y = m_meshData.vertices[2].pos.y;
		m_meshData.vertices[0].pos.z = -kPlaneHeightAboveJdgline / 2 + kPlaneHeightAboveJdgline * cosRotationX; // 奥の辺 手前方向
		m_meshData.vertices[1].pos.z = m_meshData.vertices[0].pos.z;
		m_meshData.vertices[2].pos.z = -kPlaneHeightAboveJdgline / 2 - kPlaneHeightBelowJdgline / 2 * cosRotationX - zoom * 100 * Cos(kCameraToJdglineRadians) * kPlaneHeight / kPlaneHeightAboveJdgline; // 手前の辺 手前方向
		m_meshData.vertices[3].pos.z = m_meshData.vertices[2].pos.z;

		const double rotationXMod = MathUtils::WrappedFmod(rotationX, Math::TwoPi);
		const bool shouldFlipTriangles = Math::Pi - kHighwayRotationXByCamera <= rotationXMod && rotationXMod < Math::TwoPi - kHighwayRotationXByCamera;
		if (shouldFlipTriangles != m_trianglesFlipped)
		{
			m_meshData.flipTriangles();
			m_trianglesFlipped = shouldFlipTriangles;
		}

		m_mesh.fill(m_meshData);
	}

	void Highway3DGraphics::draw2D(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext) const
	{
		m_renderTexture.drawBaseTexture(viewStatus.camStatus.centerSplit);

		// Draw shine effect
		{
			const ScopedRenderTarget2D renderTarget(m_renderTexture.additiveTexture());
			const ScopedRenderStates2D renderState(BlendState::Additive);

			const double rate = MathUtils::WrappedFmod(gameStatus.currentTimeSec, kShineEffectLoopSec) / kShineEffectLoopSec;
			const Vec2 position = kShineEffectPositionOffset + kShineEffectPositionDiff * rate;
			if (MathUtils::AlmostEquals(viewStatus.camStatus.centerSplit, 0.0))
			{
				// center_split不使用時はそのまま描画
				for (int32 i = 0; i < kNumShineEffects; ++i)
				{
					m_shineEffectTexture.draw(position + kShineEffectPositionDiff * i);
				}
			}
			else
			{
				// center_split使用時は左右に分割して描画
				const double centerSplitShiftX = Camera::CenterSplitShiftX(viewStatus.camStatus.centerSplit);
				const Size halfSize = { m_shineEffectTexture.width() / 2, m_shineEffectTexture.height() };
				for (int32 i = 0; i < kNumShineEffects; ++i)
				{
					m_shineEffectTexture(0, 0, halfSize).draw(Vec2::Right(-centerSplitShiftX) + position + kShineEffectPositionDiff * i);
					m_shineEffectTexture(halfSize.x, 0, halfSize).draw(Vec2::Right(halfSize.x + centerSplitShiftX) + position + kShineEffectPositionDiff * i);
				}
			}
		}

		// BT/FXノーツの描画
		m_buttonNoteGraphics.draw(chartData, gameStatus, viewStatus, highwayScrollContext, m_renderTexture);

		// キービームの描画
		m_keyBeamGraphics.draw(gameStatus, viewStatus, m_renderTexture);

		// レーザーノーツの描画
		m_laserNoteGraphics.draw(chartData, gameStatus, highwayScrollContext, m_renderTexture);
	}

	void Highway3DGraphics::draw3D(const GameStatus& gameStatus, const ViewStatus& viewStatus) const
	{
		// レンダーテクスチャを3D空間上へ描画
		const double radians = Math::ToRadians(viewStatus.camStatus.rotationZ) + viewStatus.tiltRadians;
		const double shiftX = viewStatus.camStatus.shiftX;
		const Transformer3D transform(Camera::CamShiftXMatrix(shiftX) * TiltTransformMatrix(radians));
		m_renderTexture.draw3D(m_mesh);
	}
}
