#include "Highway3DGraphics.hpp"
#include "MusicGame/Graphics/GraphicsDefines.hpp"
#include "MusicGame/Camera/CameraMath.hpp"
#include "Note/NoteGraphicsUtils.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kShineEffectTextureFilename = U"lanelight.gif";
		constexpr StringView kBarLineTextureFilename = U"bline.gif";

		// カメラ座標と判定ラインを線で結んだ場合の垂直からの角度
		// (値の根拠は不明だが、KSMv1でこの値が使用されていたためそのまま持ってきている)
		constexpr float kCameraToJdglineRadians = -0.6125f;

		constexpr float kPlaneHeight = kHighwayPlaneSize.y;
		constexpr float kPlaneHeightBelowJdgline = kPlaneHeight * kJdglineYFromBottom / kHighwayTextureSize.y;
		constexpr float kPlaneHeightAboveJdgline = kPlaneHeight - kPlaneHeightBelowJdgline;

		// UV座標の縮め幅(端にテクスチャの折り返しピクセルが見える現象の対策)
		constexpr float kUVShrinkX = 0.0075f;
		constexpr float kUVShrinkY = 0.005f;

		constexpr int32 kNumShineEffects = 4;
		constexpr Vec2 kShineEffectPositionOffset = kLanePositionOffset - Vec2{ 4.0, 0.0 };
		constexpr Vec2 kShineEffectPositionDiff = { 0.0, 300.0 };
		constexpr double kShineEffectLoopSec = 0.2;

		constexpr int32 kMaxNumBarLines = 50;
		constexpr Size kBarLineTextureSize = { 192, 16 };
		constexpr Size kBarLineTextureHalfSize = { kBarLineTextureSize.x / 2, kBarLineTextureSize.y };
		constexpr Vec2 kBarLinePositionOffset = { kHighwayTextureWideCenterX - kBarLineTextureHalfSize.x, -14 };
		constexpr ColorF kBarLineColor = Color{ 110 };

		// レーンが平置きからどれだけ起き上がっているかの角度
		constexpr int32 kHighwayRotationXByCameraDeg = 60;

		// レーン反転判定の角度しきい値
		constexpr int32 kFlipAngleLowerBound = 180 - kHighwayRotationXByCameraDeg;
		constexpr int32 kFlipAngleUpperBound = 360 - kHighwayRotationXByCameraDeg;

		// 角度を0〜360度に正規化
		[[nodiscard]]
		double NormalizeDegrees(double degrees)
		{
			return MathUtils::WrappedFmod(degrees, 360.0);
		}
	}

	Highway3DGraphics::Highway3DGraphics()
		: m_shineEffectTexture(TextureAsset(kShineEffectTextureFilename))
		, m_barLineTexture(TextureAsset(kBarLineTextureFilename))
		, m_meshData(MeshData::Grid({ 0.0, 0.0, 0.0 }, kHighwayPlaneSizeWide, 1, 1, { 1.0f - kUVShrinkX, 1.0f - kUVShrinkY }, { kUVShrinkX / 2, kUVShrinkY / 2 }))
		, m_mesh(m_meshData) // DynamicMesh::fill()で頂点データの配列サイズが動的に変更される訳ではないのでこの初期化は必須
	{
	}

	void Highway3DGraphics::update(const ViewStatus& viewStatus)
	{
		// メッシュの頂点座標を更新
		// HSP版の該当箇所: https://github.com/kshootmania/ksm-v1/blob/d2811a09e2d75dad5cc152d7c4073897061addb7/src/scene/play/play_draw_frame.hsp#L779-L821

		const auto& camStatus = viewStatus.camStatus;
		const float scaledZoomBottom = static_cast<float>(Camera::ScaledCamZoomBottomValue(camStatus.zoomBottom));

		if (camStatus.useLegacyZoomTop)
		{
			// KSHバージョン167未満の場合、zoom_topは移動として扱う
			// HSP版: https://github.com/kshootmania/ksm-v1/blob/ea05374a3ece796612b29d927cb3c6f5aabb266e/src/scene/play/play_draw_frame.hsp#L828-L841
			const float legacyZoomTop = static_cast<float>(camStatus.zoomTop);
			const float heightRatio = kPlaneHeightBelowJdgline / kPlaneHeightAboveJdgline; // sr/lr

			m_meshData.vertices[0].pos.y = -legacyZoomTop * 100 * Sin(kCameraToJdglineRadians); // 奥の辺 上方向
			m_meshData.vertices[1].pos.y = m_meshData.vertices[0].pos.y;
			m_meshData.vertices[0].pos.z = kPlaneHeight / 2 + legacyZoomTop * 100 * Cos(kCameraToJdglineRadians); // 奥の辺 手前方向(v1とZ軸の向きが逆なので符号を反転)
			m_meshData.vertices[1].pos.z = m_meshData.vertices[0].pos.z;

			m_meshData.vertices[2].pos.y = -scaledZoomBottom * 100 * Sin(kCameraToJdglineRadians) * kPlaneHeight / kPlaneHeightAboveJdgline // 手前の辺 上方向
			                              + legacyZoomTop * 100 * heightRatio * Sin(kCameraToJdglineRadians);
			m_meshData.vertices[3].pos.y = m_meshData.vertices[2].pos.y;
			m_meshData.vertices[2].pos.z = -kPlaneHeight / 2 // 手前の辺 手前方向(v1とZ軸の向きが逆なので符号を反転)
			                              - scaledZoomBottom * 100 * Cos(kCameraToJdglineRadians) * kPlaneHeight / kPlaneHeightAboveJdgline
			                              - legacyZoomTop * 100 * heightRatio * Cos(kCameraToJdglineRadians);
			m_meshData.vertices[3].pos.z = m_meshData.vertices[2].pos.z;
		}
		else
		{
			// KSHバージョン167以降の場合、zoom_topは判定ラインまわりの回転として扱う
			const float rotationX = static_cast<float>(ToRadians(camStatus.zoomTop * 360 / 2400));
			const float sinRotationX = Sin(rotationX);
			const float cosRotationX = Cos(rotationX);

			m_meshData.vertices[0].pos.y = kPlaneHeightAboveJdgline * sinRotationX / 2.5f; // 奥の辺 上方向
			m_meshData.vertices[1].pos.y = m_meshData.vertices[0].pos.y;
			m_meshData.vertices[2].pos.y = -scaledZoomBottom * 100 * Sin(kCameraToJdglineRadians) * kPlaneHeight / kPlaneHeightAboveJdgline - kPlaneHeightBelowJdgline * sinRotationX / 2.5f; // 手前の辺 上方向
			m_meshData.vertices[3].pos.y = m_meshData.vertices[2].pos.y;
			m_meshData.vertices[0].pos.z = -kPlaneHeightAboveJdgline / 2 + kPlaneHeightAboveJdgline * cosRotationX; // 奥の辺 手前方向(v1とZ軸の向きが逆なので符号を反転)
			m_meshData.vertices[1].pos.z = m_meshData.vertices[0].pos.z;
			m_meshData.vertices[2].pos.z = -kPlaneHeightAboveJdgline / 2 - kPlaneHeightBelowJdgline / 2 * cosRotationX - scaledZoomBottom * 100 * Cos(kCameraToJdglineRadians) * kPlaneHeight / kPlaneHeightAboveJdgline; // 手前の辺 手前方向(v1とZ軸の向きが逆なので符号を反転)
			m_meshData.vertices[3].pos.z = m_meshData.vertices[2].pos.z;

			// 背面カリングを使用しているため、反転時は頂点の順序を入れ替える
			// v1と同様に整数角度で判定(zoom_top=-400の境界条件を変えないため浮動小数点数で判定しない)
			const int32 rotationXDeg = static_cast<int32>(NormalizeDegrees(ToDegrees(rotationX)));
			const bool shouldFlipTriangles = kFlipAngleLowerBound <= rotationXDeg && rotationXDeg < kFlipAngleUpperBound;
			if (shouldFlipTriangles != m_trianglesFlipped)
			{
				m_meshData.flipTriangles();
				m_trianglesFlipped = shouldFlipTriangles;
			}
		}

		m_mesh.fill(m_meshData);
	}

	void Highway3DGraphics::draw2D(const kson::ChartData& chartData, const std::array<HashSet<kson::Pulse>, kson::kNumLaserLanesSZ>& laserCurvedPulses, const PlayOption& playOption, const kson::TimingCache& timingCache, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext) const
	{
		m_renderTexture.drawBaseTexture(viewStatus.camStatus.centerSplit);

		{
			const ScopedRenderTarget2D renderTarget(m_renderTexture.additiveTexture());
			const ScopedRenderStates2D renderState(BlendState::Additive);

			const bool isCenterSplitUsed = !MathUtils::AlmostEquals(viewStatus.camStatus.centerSplit, 0.0);
			const double centerSplitShiftX = isCenterSplitUsed ? Camera::CenterSplitShiftX(viewStatus.camStatus.centerSplit) : 0.0;

			// 光沢エフェクトを描画
			{

				const double rate = MathUtils::WrappedFmod(gameStatus.currentTimeSec, kShineEffectLoopSec) / kShineEffectLoopSec;
				const Vec2 position = kShineEffectPositionOffset + kShineEffectPositionDiff * rate;
				if (isCenterSplitUsed)
				{
					// center_split使用時は左右に分割して描画
					const Size halfSize = { m_shineEffectTexture.width() / 2, m_shineEffectTexture.height() };
					for (int32 i = 0; i < kNumShineEffects; ++i)
					{
						m_shineEffectTexture(0, 0, halfSize).draw(Vec2::Right(-centerSplitShiftX) + position + kShineEffectPositionDiff * i);
						m_shineEffectTexture(halfSize.x, 0, halfSize).draw(Vec2::Right(halfSize.x + centerSplitShiftX) + position + kShineEffectPositionDiff * i);
					}
				}
				else
				{
					// center_split不使用時はそのまま描画
					for (int32 i = 0; i < kNumShineEffects; ++i)
					{
						m_shineEffectTexture.draw(position + kShineEffectPositionDiff * i);
					}
				}
			}

			// 小節線を描画
			{
				const int64 startMeasureIndex = kson::PulseToMeasureIdx(gameStatus.currentPulse, chartData.beat, timingCache) - 1;
				const int64 endMeasureIndex = startMeasureIndex + kMaxNumBarLines + 1;
				for (int64 measureIndex = startMeasureIndex; measureIndex < endMeasureIndex; ++measureIndex)
				{
					const kson::Pulse pulse = kson::MeasureIdxToPulse(measureIndex, chartData.beat, timingCache);
					const int32 pulsePositionY = highwayScrollContext.getPositionY(pulse);
					const int32 actualPositionY = static_cast<int32>(kBarLinePositionOffset.y) + pulsePositionY;
					if (actualPositionY >= kHighwayTextureSize.y)
					{
						continue;
					}
					if (actualPositionY + kBarLineTextureSize.y < 0)
					{
						break;
					}

					const Vec2 position = kBarLinePositionOffset + Vec2::Down(pulsePositionY);
					if (isCenterSplitUsed)
					{
						// center_split使用時は左右に分割して描画
						m_barLineTexture(0, 0, kBarLineTextureHalfSize).draw(Vec2::Right(-centerSplitShiftX) + position, kBarLineColor);
						m_barLineTexture(kBarLineTextureHalfSize.x, 0, kBarLineTextureHalfSize).draw(Vec2::Right(kBarLineTextureHalfSize.x + centerSplitShiftX) + position, kBarLineColor);
					}
					else
					{
						// center_split不使用時はそのまま描画
						m_barLineTexture.draw(position, kBarLineColor);
					}
				}
			}
		}

		// BT/FXノーツの描画
		m_buttonNoteGraphics.draw(chartData, gameStatus, viewStatus, playOption, highwayScrollContext, m_renderTexture);

		// キービームの描画
		m_keyBeamGraphics.draw(gameStatus, viewStatus, m_renderTexture);

		// レーザーノーツの描画
		m_laserNoteGraphics.draw(chartData, laserCurvedPulses, playOption, gameStatus, highwayScrollContext, m_renderTexture);
	}

	void Highway3DGraphics::draw3D(const ViewStatus& viewStatus) const
	{
		// レンダーテクスチャを3D空間上へ描画
		const double radians = Math::ToRadians(viewStatus.camStatus.rotationDeg + viewStatus.camStatus.rotationDegHighway) + viewStatus.tiltRadians;
		const double shiftX = viewStatus.camStatus.shiftX;
		const Transformer3D transform(Camera::CamShiftXMatrix(shiftX) * TiltTransformMatrix(radians));
		m_renderTexture.draw3D(m_mesh);
	}
}
