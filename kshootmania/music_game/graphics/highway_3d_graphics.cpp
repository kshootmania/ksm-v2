#include "highway_3d_graphics.hpp"

namespace
{
	constexpr FilePathView kHighwayBGTexturePath = U"imgs/base.gif";
	constexpr FilePathView kShineEffectTexturePath = U"imgs/lanelight.gif";
	constexpr FilePathView kKeyBeamTexturePath = U"imgs/judge.gif";
	constexpr FilePathView kChipBTNoteTexturePath = U"imgs/bt_chip.gif";
	constexpr FilePathView kLongBTNoteTexturePath = U"imgs/bt_long.gif";
	constexpr FilePathView kChipFXNoteTexturePath = U"imgs/fx_chip.gif";
	constexpr FilePathView kLongFXNoteTexturePath = U"imgs/fx_long.gif";

	// Angle of the line connecting the camera position and the judgment line from the horizontal
	// (Unconfirmed, but KSMv1 uses this value anyway)
	constexpr double kCameraToJudgmentLineRadians = -0.6125;

	constexpr Size kTextureSize = { 256, 1024 };
	constexpr double kJudgmentLineYFromBottom = 14;

	constexpr double kPlaneWidth = 304.0 * 2 / 13;
	constexpr double kPlaneHeight = 936.0 * 13 / 20;
	constexpr double kPlaneHeightBelowJudgmentLine = kPlaneHeight * kJudgmentLineYFromBottom / kTextureSize.y;
	constexpr double kPlaneHeightAboveJudgmentLine = kPlaneHeight - kPlaneHeightBelowJudgmentLine;

	// Shrink UV to remove edge pixels
	constexpr float kUVShrinkX = 0.0075f;
	constexpr float kUVShrinkY = 0.005f;

	constexpr Vec2 kLanePositionOffset = { 44.0, 0.0 };
	constexpr Vec2 kBTLanePositionDiff = { 42.0, 0.0 };
	constexpr Vec2 kFXLanePositionDiff = { 84.0, 0.0 };
	constexpr Size kBTKeyBeamTextureSize = { 40, 300 };
	constexpr Size kFXKeyBeamTextureSize = { 82, 300 };

	constexpr int32 kNumShineEffects = 4;
	constexpr Vec2 kShineEffectPositionOffset = { 40.0, 0.0 };
	constexpr Vec2 kShineEffectPositionDiff = { 0.0, 300.0 };

	constexpr double kKeyBeamFullWidthSec = 0.075;
	constexpr double kKeyBeamEndSec = 0.155;
	constexpr Vec2 kKeyBeamPositionOffset = kLanePositionOffset + Vec2{ 0.0, kTextureSize.y - 300.0 };
}

MusicGame::Graphics::Highway3DGraphics::Highway3DGraphics()
	: m_bgTexture(kHighwayBGTexturePath, TextureDesc::UnmippedSRGB)
	, m_shineEffectTexture(kShineEffectTexturePath, TextureDesc::UnmippedSRGB)
	, m_beamTexture(kKeyBeamTexturePath, TextureDesc::UnmippedSRGB)
	, m_chipBTNoteTexture(Texture(kChipBTNoteTexturePath, TextureDesc::UnmippedSRGB),
		{
			.column = 9 * 2,
			.sourceSize = { 40, 14 },
		})
	, m_longBTNoteTexture(kLongBTNoteTexturePath, TextureDesc::UnmippedSRGB)
	, m_chipFXNoteTexture(Texture(kChipFXNoteTexturePath, TextureDesc::UnmippedSRGB),
		{
			.column = 2,
			.sourceSize = { 82, 14 },
		})
	, m_longFXNoteTexture(kLongFXNoteTexturePath, TextureDesc::UnmippedSRGB)
	, m_additiveRenderTexture(kTextureSize, TextureFormat::R8G8B8A8_Unorm_SRGB)
	, m_subtractiveRenderTexture(kTextureSize, TextureFormat::R8G8B8A8_Unorm_SRGB)
	, m_meshData(MeshData::Grid({ 0.0, 0.0, 0.0 }, { kPlaneWidth, kPlaneHeight }, 2, 2, { 1.0f - kUVShrinkX, 1.0f - kUVShrinkY }, { kUVShrinkX / 2, kUVShrinkY / 2 }))
	, m_mesh(m_meshData) // <- this initialization is important because DynamicMesh::fill() does not dynamically resize the vertex array
{
}

void MusicGame::Graphics::Highway3DGraphics::update(const UpdateInfo& updateInfo)
{
	// TODO: Calculate vertex position from cameraState

	m_updateInfo = updateInfo;

	assert(m_updateInfo.pChartData != nullptr);
}

void MusicGame::Graphics::Highway3DGraphics::draw(const RenderTexture& target) const
{
	const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);
	Shader::Copy(m_bgTexture(0, 0, kTextureSize), m_additiveRenderTexture);
	Shader::Copy(m_bgTexture(kTextureSize.x, 0, kTextureSize), m_subtractiveRenderTexture);

	// Draw shine effect
	{
		const ScopedRenderTarget2D renderTarget(m_additiveRenderTexture);
		const ScopedRenderStates2D renderState(BlendState::Additive);

		for (int32 i = 0; i < kNumShineEffects; ++i)
		{
			m_shineEffectTexture.draw(kShineEffectPositionOffset + kShineEffectPositionDiff * i + kShineEffectPositionDiff * MathUtils::WrappedFmod(m_updateInfo.currentTimeSec, 0.2) / 0.2);
		}
	}

	// Draw notes
	if (m_updateInfo.pChartData != nullptr)
	{
		const ScopedRenderTarget2D renderTarget(m_additiveRenderTexture);
		const ScopedRenderStates2D renderState(BlendState::Additive);

		const ksh::ChartData& chartData = *m_updateInfo.pChartData;

		// BT notes
		for (std::size_t laneIdx = 0; laneIdx < ksh::kNumBTLanes; ++laneIdx)
		{
			const auto& lane = chartData.note.btLanes[laneIdx];
			for (const auto& [y, note] : lane)
			{
				if (y + note.length < m_updateInfo.currentPulse - chartData.beat.resolution)
				{
					continue;
				}

				const double positionStartY = static_cast<double>(kTextureSize.y) - static_cast<double>(y - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution;
				if (positionStartY < 0)
				{
					break;
				}

				if (note.length == 0)
				{
					// Chip BT notes
					m_chipBTNoteTexture().draw(kLanePositionOffset + kBTLanePositionDiff * laneIdx + Vec2::Down(positionStartY));
				}
				else
				{
					// Long BT notes
					const double positionEndY = static_cast<double>(kTextureSize.y) - static_cast<double>(y + note.length - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution;
					m_longBTNoteTexture(0, 0, 40, 1)
						.resized(40, note.length * 480 / chartData.beat.resolution)
						.draw(kLanePositionOffset + kBTLanePositionDiff * laneIdx + Vec2::Down(positionEndY));
				}
			}
		}

		// FX notes
		for (std::size_t laneIdx = 0; laneIdx < ksh::kNumFXLanes; ++laneIdx)
		{
			const auto& lane = chartData.note.fxLanes[laneIdx];
			for (const auto& [y, note] : lane)
			{
				if (y + note.length < m_updateInfo.currentPulse - chartData.beat.resolution)
				{
					continue;
				}

				const double positionStartY = static_cast<double>(kTextureSize.y) - static_cast<double>(y - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution;
				if (positionStartY < 0)
				{
					break;
				}

				if (note.length == 0)
				{
					// Chip FX notes
					m_chipFXNoteTexture().draw(kLanePositionOffset + kFXLanePositionDiff * laneIdx + Vec2::Down(positionStartY));
				}
				else
				{
					// Long FX notes
					const double positionEndY = static_cast<double>(kTextureSize.y) - static_cast<double>(y + note.length - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution;
					m_longFXNoteTexture(0, 0, 82, 1)
						.resized(82, note.length * 480 / chartData.beat.resolution)
						.draw(kLanePositionOffset + kFXLanePositionDiff * laneIdx + Vec2::Down(positionEndY));
				}
			}
		}
	}

	// Draw key beam
	{
		const ScopedRenderTarget2D renderTarget(m_additiveRenderTexture);
		const ScopedRenderStates2D renderState(BlendState::Additive);

		for (std::size_t i = 0; i < ksh::kNumBTLanes + ksh::kNumFXLanes; ++i)
		{
			const bool isBT = (i < ksh::kNumBTLanes);
			const std::size_t laneIdx = isBT ? i : (i - ksh::kNumBTLanes);
			const LaneState& laneState = isBT ? m_updateInfo.btLaneState[laneIdx] : m_updateInfo.fxLaneState[laneIdx];

			const double sec = m_updateInfo.currentTimeSec - laneState.keyBeamTimeSec;
			if (sec < 0.0 || kKeyBeamEndSec < sec)
			{
				continue;
			}

			double widthRate = 1.0;
			double alpha = 1.0;
			if (sec < kKeyBeamFullWidthSec)
			{
				widthRate = sec / kKeyBeamFullWidthSec;
			}
			else
			{
				alpha = 1.0 - (sec - kKeyBeamFullWidthSec) / (kKeyBeamEndSec - kKeyBeamFullWidthSec);
			}

			const TextureRegion beamTextureRegion = m_beamTexture(
				kBTKeyBeamTextureSize.x * (laneState.keyBeamType + 0.5 - widthRate / 2),
				0,
				kBTKeyBeamTextureSize.x * widthRate,
				kBTKeyBeamTextureSize.y);

			if (isBT)
			{
				beamTextureRegion
					.draw(kKeyBeamPositionOffset + kBTLanePositionDiff * (laneIdx + 0.5 - widthRate / 2), ColorF{ 1.0, alpha });
			}
			else
			{
				beamTextureRegion
					.resized(kFXKeyBeamTextureSize.x * widthRate, kFXKeyBeamTextureSize.y)
					.draw(kKeyBeamPositionOffset + kFXLanePositionDiff * laneIdx + kFXKeyBeamTextureSize * (0.5 - widthRate / 2), ColorF{ 1.0, alpha });
			}
		}
	}

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
