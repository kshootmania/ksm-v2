#include "jdgoverlay_3d_graphics.hpp"
#include "music_game/graphics/graphics_defines.hpp"
#include "music_game/camera/camera_math.hpp"
#include "kson/common/common.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr Size kTextureSize = { 452 * 2, 140 };

		constexpr Float3 kPlaneCenter = { 0.0f, 3.5f, -kHighwayPlaneSize.y / 2 - 1.8f };
		constexpr Float2 kPlaneSize = { kTextureSize.x * 0.95f / 8, kTextureSize.y * 0.95f / 8 };

		constexpr StringView kChipCriticalAnimTextureFilename = U"judge1.gif";
		constexpr StringView kChipNearAnimTextureFilename = U"judge2.gif";
		constexpr StringView kChipErrorAnimTextureFilename = U"judge3.gif";
		constexpr int32 kChipAnimFrames = 12;
		constexpr double kChipAnimDurationSec = 0.5;
		constexpr Size kChipAnimSourceSize = { 150, 150 };
		constexpr Size kChipAnimSize = { 86, 86 };

		constexpr StringView kLongAnimTextureFilename = U"judgelong.gif";
		constexpr int32 kLongAnimStartFrames = 8;
		constexpr double kLongAnimStartDurationSec = 0.5;
		constexpr int32 kLongAnimLoopFrameOffset = kLongAnimStartFrames;
		constexpr int32 kLongAnimLoopFrames = 26;
		constexpr double kLongAnimLoopDurationSec = 1.625;
		constexpr int32 kLongAnimEndFrameOffset = kLongAnimLoopFrameOffset + kLongAnimLoopFrames;
		constexpr int32 kLongAnimEndFrames = 8;
		constexpr double kLongAnimEndDurationSec = 0.5;
		constexpr int32 kLongAnimTotalFrames = kLongAnimStartFrames + kLongAnimLoopFrames + kLongAnimEndFrames;
		constexpr Size kLongAnimSourceSize = { 300, 300 };
		constexpr Size kLongAnimSizeBT = { 120, 120 };
		constexpr Size kLongAnimSizeFX = { 140, 140 };

		constexpr StringView kLaserAnimTextureFilename = U"judgelaser.gif";
		constexpr double kLaserAnimLoopDurationSec = 1.2;
		constexpr int32 kLaserAnimLoopFrames = 26;
		constexpr Size kLaserAnimSourceSize = { 150, 150 };
		constexpr Size kLaserAnimSize = { 100, 100 };
	}

	const TiledTexture& Jdgoverlay3DGraphics::chipAnimTexture(Judgment::JudgmentResult type) const
	{
		using enum Judgment::JudgmentResult;

		switch (type)
		{
		case kCritical:
			return m_chipCriticalTexture;

		case kNear:
			return m_chipNearTexture; // TODO: Fast near

		case kError:
			return m_chipErrorTexture;

		default:
			assert(false);
			return m_chipErrorTexture;
		}
	}

	void Jdgoverlay3DGraphics::drawChipAnimCommon(const GameStatus& gameStatus, bool isBT) const
	{
		const int32 numLanes = isBT ? kson::kNumBTLanes : kson::kNumFXLanes;
		for (int32 i = 0; i < numLanes; ++i)
		{
			const auto& laneStatus = isBT ? gameStatus.btLaneStatus[i] : gameStatus.fxLaneStatus[i];
			for (const auto& chipAnimState : laneStatus.chipAnimStatusRingBuffer)
			{
				const double sec = gameStatus.currentTimeSec - chipAnimState.startTimeSec;
				if (0.0 <= sec && sec < kChipAnimDurationSec && chipAnimState.type != Judgment::JudgmentResult::kUnspecified)
				{
					const int32 frameIdx = static_cast<int32>(sec / kChipAnimDurationSec * kChipAnimFrames);
					const Vec2 position = ScreenUtils::Scaled(kTextureSize.x / 4 + 92 + (isBT ? 0 : 30) + (isBT ? 60 : 120) * i, 17);
					chipAnimTexture(chipAnimState.type)(frameIdx).resized(ScreenUtils::Scaled(kChipAnimSize)).draw(position);
				}
			}
		}
	}

	void Jdgoverlay3DGraphics::drawChipAnimBT(const GameStatus& gameStatus) const
	{
		drawChipAnimCommon(gameStatus, true);
	}

	void Jdgoverlay3DGraphics::drawChipAnimFX(const GameStatus& gameStatus) const
	{
		drawChipAnimCommon(gameStatus, false);
	}

	void Jdgoverlay3DGraphics::drawLongAnimCommon(const GameStatus& gameStatus, bool isBT) const
	{
		const int32 numLanes = isBT ? kson::kNumBTLanes : kson::kNumFXLanes;
		for (int32 i = 0; i < numLanes; ++i)
		{
			const auto& laneStatus = isBT ? gameStatus.btLaneStatus[i] : gameStatus.fxLaneStatus[i];
			const double sec = gameStatus.currentTimeSec - laneStatus.currentLongNoteAnimOffsetTimeSec;
			const Vec2 position = ScreenUtils::Scaled(kTextureSize.x / 4 + (isBT ? 75 : 96) + (isBT ? 60 : 120) * i, isBT ? 10 : 0);
			const SizeF size = ScreenUtils::Scaled(isBT ? kLongAnimSizeBT : kLongAnimSizeFX);

			int32 frameIdx;
			if (laneStatus.currentLongNotePulse.has_value())
			{
				if (0.0 <= sec && sec < kChipAnimDurationSec)
				{
					// ロングノーツ押下開始時のアニメーション
					frameIdx = static_cast<int32>(sec / kLongAnimStartDurationSec * kLongAnimStartFrames);
				}
				else
				{
					// ロングノーツ押下中のループアニメーション
					frameIdx = kLongAnimLoopFrameOffset + static_cast<int32>(MathUtils::WrappedFmod(sec / kLongAnimLoopDurationSec, 1.0) * kLongAnimLoopFrames);
					m_longAnimTexture(frameIdx).resized(size).draw(position);
				}
			}
			else if (0.0 <= sec && sec < kChipAnimDurationSec)
			{
				// ロングノーツ終了時または途中で離したときのアニメーション
				frameIdx = kLongAnimEndFrameOffset + static_cast<int32>(sec / kLongAnimEndDurationSec * kLongAnimEndFrames);
			}
			else
			{
				continue;
			}

			m_longAnimTexture(frameIdx).resized(size).draw(position);
		}
	}

	void Jdgoverlay3DGraphics::drawLongAnimBT(const GameStatus& gameStatus) const
	{
		drawLongAnimCommon(gameStatus, true);
	}

	void Jdgoverlay3DGraphics::drawLongAnimFX(const GameStatus& gameStatus) const
	{
		drawLongAnimCommon(gameStatus, false);
	}

	void Jdgoverlay3DGraphics::drawLaserAnim(const GameStatus& gameStatus) const
	{
		const int32 frameIdx = static_cast<int32>(MathUtils::WrappedFmod(gameStatus.currentTimeSec / kLaserAnimLoopDurationSec, 1.0) * kLaserAnimLoopFrames);
		const SizeF size = ScreenUtils::Scaled(kLaserAnimSize);
		for (int32 i = 0; i < kson::kNumLaserLanes; ++i)
		{
			const auto& laneStatus = gameStatus.laserLaneStatus[i];
			if (!laneStatus.isCursorInCriticalJudgmentRange())
			{
				// カーソルがクリティカル判定の範囲内でない
				continue;
			}
			const double cursorX = laneStatus.cursorWide ? ((laneStatus.cursorX.value() - 0.5) * 2 + 0.5) : laneStatus.cursorX.value();
			const Vec2 position = ScreenUtils::Scaled(kTextureSize.x / 4 + 28 + static_cast<int32>(295 * cursorX), 17);
			m_laserAnimTexture(frameIdx, i).resized(size).draw(position);
		}
	}

	Jdgoverlay3DGraphics::Jdgoverlay3DGraphics(const BasicCamera3D& camera)
		: m_renderTexture(ScreenUtils::Scaled(kTextureSize.x), ScreenUtils::Scaled(kTextureSize.y))
		, m_chipCriticalTexture(kChipCriticalAnimTextureFilename,
			{
				.row = kChipAnimFrames,
				.sourceScale = ScreenUtils::SourceScale::kNoScaling,
				.sourceSize = kChipAnimSourceSize,
			})
		, m_chipNearTexture(kChipNearAnimTextureFilename,
			{
				.row = kChipAnimFrames,
				.column = 2, // 1列目はSLOW(またはFAST/SLOW非表示時)、2列目はFAST
				.sourceScale = ScreenUtils::SourceScale::kNoScaling,
				.sourceSize = kChipAnimSourceSize,
			})
		, m_chipErrorTexture(kChipErrorAnimTextureFilename,
			{
				.row = kChipAnimFrames, // TODO: FAST
				.sourceScale = ScreenUtils::SourceScale::kNoScaling,
				.sourceSize = kChipAnimSourceSize,
			})
		, m_longAnimTexture(kLongAnimTextureFilename,
			{
				.row = kLongAnimTotalFrames,
				.sourceScale = ScreenUtils::SourceScale::kNoScaling,
				.sourceSize = kLongAnimSourceSize,
			})
		, m_laserAnimTexture(kLaserAnimTextureFilename,
			{
				.row = kLaserAnimLoopFrames,
				.column = kson::kNumLaserLanes,
				.sourceScale = ScreenUtils::SourceScale::kNoScaling,
				.sourceSize = kLaserAnimSourceSize,
			})
		, m_transform(camera.billboard(kPlaneCenter, kPlaneSize))
		, m_mesh(MeshData::Billboard())
	{
	}

	void Jdgoverlay3DGraphics::draw2D(const GameStatus& gameStatus) const
	{
		// 描画するレンダーテクスチャを用意
		const ScopedRenderTarget2D renderTarget(m_renderTexture.clear(Palette::Black));
		const ScopedRenderStates2D blendState(BlendState::Additive);
		drawChipAnimBT(gameStatus);
		drawChipAnimFX(gameStatus);
		drawLongAnimBT(gameStatus);
		drawLongAnimFX(gameStatus);
		drawLaserAnim(gameStatus);
	}

	void Jdgoverlay3DGraphics::draw3D(const ViewStatus& viewStatus) const
	{
		// レンダーテクスチャを3D空間上に描画
		const ScopedRenderStates3D blendState(BlendState::Additive);
		const double scale = Camera::JdgoverlayScale(viewStatus.camStatus.zoom);
		const Transformer3D transform(Mat4x4::Scale(scale, kPlaneCenter) * TiltTransformMatrix(viewStatus.tiltRadians));
		m_mesh.draw(m_transform, m_renderTexture);
	}
}
