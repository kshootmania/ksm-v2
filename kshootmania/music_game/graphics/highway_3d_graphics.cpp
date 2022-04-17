#include "highway_3d_graphics.hpp"
#include "graphics_defines.hpp"
#include "note/note_graphics_utils.hpp"
#include "ksh/util/graph_utils.hpp"

namespace
{
	using namespace MusicGame::Graphics;

	constexpr StringView kHighwayBGTextureFilename = U"base.gif";
	constexpr StringView kShineEffectTextureFilename = U"lanelight.gif";
	constexpr StringView kKeyBeamTextureFilename = U"judge.gif";
	constexpr StringView kLaserNoteTextureFilename = U"laser.gif";
	constexpr StringView kLaserNoteMaskTextureFilename = U"laser_mask.gif";
	constexpr StringView kLaserNoteLeftStartTextureFilename = U"laserl_0.gif";
	constexpr StringView kLaserNoteRightStartTextureFilename = U"laserr_0.gif";

	// Angle of the line connecting the camera position and the judgment line from the horizontal
	// (Unconfirmed, but KSMv1 uses this value anyway)
	constexpr double kCameraToJudgmentLineRadians = -0.6125;

	constexpr double kJudgmentLineYFromBottom = 14;

	constexpr double kPlaneWidth = 304.0 * 2 / 13;
	constexpr double kPlaneHeight = 936.0 * 13 / 20;
	constexpr double kPlaneHeightBelowJudgmentLine = kPlaneHeight * kJudgmentLineYFromBottom / kHighwayTextureSize.y;
	constexpr double kPlaneHeightAboveJudgmentLine = kPlaneHeight - kPlaneHeightBelowJudgmentLine;

	// Shrink UV to remove edge pixels
	constexpr float kUVShrinkX = 0.0075f;
	constexpr float kUVShrinkY = 0.005f;

	constexpr Size kBTKeyBeamTextureSize = { 40, 300 };
	constexpr Size kFXKeyBeamTextureSize = { 82, 300 };

	constexpr Size kLaserTextureSize = { 48, 48 };
	constexpr double kLaserLineWidth = static_cast<double>(kLaserTextureSize.x);
	constexpr Size kLaserStartTextureSize = { 44, 200 };

	constexpr int32 kNumShineEffects = 4;
	constexpr Vec2 kShineEffectPositionOffset = { 40.0, 0.0 };
	constexpr Vec2 kShineEffectPositionDiff = { 0.0, 300.0 };

	constexpr double kKeyBeamFullWidthSec = 0.075;
	constexpr double kKeyBeamEndSec = 0.155;
	constexpr Vec2 kKeyBeamPositionOffset = kLanePositionOffset + Vec2{ 0.0, kHighwayTextureSize.y - 300.0 };

	Quad LaserLineQuad(const Vec2& positionStart, const Vec2& positionEnd)
	{
		return Quad{
			positionStart + Vec2{ kLaserLineWidth / 2, 0.0 },
			positionStart + Vec2{ -kLaserLineWidth / 2, 0.0 },
			positionEnd + Vec2{ -kLaserLineWidth / 2, 0.0 },
			positionEnd + Vec2{ kLaserLineWidth / 2, 0.0 }
		};
	}

	Quad LaserSlamLineQuad(const Vec2& positionStart, const Vec2& positionEnd)
	{
		if (Abs(positionEnd.x - positionStart.x) <= kLaserLineWidth)
		{
			// Too short to draw line
			return Quad{ Vec2::Zero(), Vec2::Zero(), Vec2::Zero(), Vec2::Zero() };
		}

		const int diffXSign = Sign(positionEnd.x - positionStart.x);
		return Quad{
			positionStart + Vec2{ diffXSign * kLaserLineWidth / 2, -kLaserLineWidth },
			positionEnd + Vec2{ -diffXSign * kLaserLineWidth / 2, -kLaserLineWidth },
			positionEnd + Vec2{ -diffXSign * kLaserLineWidth / 2, 0.0 },
			positionStart + Vec2{ diffXSign * kLaserLineWidth / 2, 0.0 }
		};
	}
}

MusicGame::Graphics::Highway3DGraphics::Highway3DGraphics()
	: m_bgTexture(TextureAsset(kHighwayBGTextureFilename))
	, m_shineEffectTexture(TextureAsset(kShineEffectTextureFilename))
	, m_beamTexture(TextureAsset(kKeyBeamTextureFilename))
	, m_laserNoteTexture(TextureAsset(kLaserNoteTextureFilename))
	, m_laserNoteMaskTexture(TextureAsset(kLaserNoteMaskTextureFilename))
	, m_laserNoteLeftStartTexture(kLaserNoteLeftStartTextureFilename,
		{
			.column = 2,
			.sourceSize = kLaserStartTextureSize,
		})
	, m_laserNoteRightStartTexture(kLaserNoteRightStartTextureFilename,
		{
			.column = 2,
			.sourceSize = kLaserStartTextureSize,
		})
	, m_additiveRenderTexture(kHighwayTextureSize)
	, m_invMultiplyRenderTexture(kHighwayTextureSize)
	, m_drawTexturePairs{
		std::make_pair(std::ref(m_additiveRenderTexture), std::ref(m_laserNoteTexture)),
		std::make_pair(std::ref(m_invMultiplyRenderTexture), std::ref(m_laserNoteMaskTexture)) }
	, m_meshData(MeshData::Grid({ 0.0, 0.0, 0.0 }, { kPlaneWidth, kPlaneHeight }, 2, 2, { 1.0f - kUVShrinkX, 1.0f - kUVShrinkY }, { kUVShrinkX / 2, kUVShrinkY / 2 }))
	, m_mesh(m_meshData) // <- this initialization is important because DynamicMesh::fill() does not dynamically resize the vertex array
{
}

void MusicGame::Graphics::Highway3DGraphics::update(const UpdateInfo& updateInfo)
{
	double tiltFactor = 0.0;
	if (updateInfo.pChartData != nullptr)
	{
		const ksh::ChartData& chartData = *updateInfo.pChartData;
		const double leftLaserValue = ksh::GraphSectionValueAtWithDefault(chartData.note.laserLanes[0], updateInfo.currentPulse, 0.0); // range: [0, +1]
		const double rightLaserValue = ksh::GraphSectionValueAtWithDefault(chartData.note.laserLanes[1], updateInfo.currentPulse, 1.0) - 1.0; // range: [-1, 0]
		tiltFactor = leftLaserValue + rightLaserValue; // range: [-1, +1]
	}
	m_highwayTilt.update(tiltFactor);
}

void MusicGame::Graphics::Highway3DGraphics::draw(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const
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

	// Draw key beam
	{
		const ScopedRenderTarget2D renderTarget(m_additiveRenderTexture);
		const ScopedRenderStates2D renderState(BlendState::Additive);

		for (std::size_t i = 0; i < ksh::kNumBTLanes + ksh::kNumFXLanes; ++i)
		{
			const bool isBT = (i < ksh::kNumBTLanes);
			const std::size_t laneIdx = isBT ? i : (i - ksh::kNumBTLanes);
			const LaneState& laneState = isBT ? updateInfo.btLaneState[laneIdx] : updateInfo.fxLaneState[laneIdx];

			const double sec = updateInfo.currentTimeSec - laneState.keyBeamTimeSec;
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

	// Draw laser notes
	if (updateInfo.pChartData != nullptr)
	{
		const ScopedRenderStates2D renderState(BlendState::Additive);

		const ksh::ChartData& chartData = *updateInfo.pChartData;

		for (std::size_t laneIdx = 0; laneIdx < ksh::kNumLaserLanes; ++laneIdx)
		{
			const auto& lane = chartData.note.laserLanes[laneIdx];
			for (const auto& [y, laserSection] : lane)
			{
				const double positionSectionStartY = static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(y - updateInfo.currentPulse) * 480 / chartData.beat.resolution;
				if (positionSectionStartY + kLaserStartTextureSize.y < 0)
				{
					// Laser section is above the drawing area
					break;
				}

				for (auto itr = laserSection.points.begin(); itr != laserSection.points.end(); ++itr)
				{
					const auto& [ry, point] = *itr;

					// Draw laser start texture
					if (itr == laserSection.points.begin())
					{
						for (int32 i = 0; i < 2; ++i)
						{
							const ScopedRenderTarget2D renderTarget((i == 0) ? m_additiveRenderTexture : m_invMultiplyRenderTexture);
							const Vec2 positionStart = {
								point.v * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
								static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(y + ry - updateInfo.currentPulse) * 480 / chartData.beat.resolution
							};
							const TiledTexture& startTexture = (laneIdx == 0) ? m_laserNoteLeftStartTexture : m_laserNoteRightStartTexture;
							startTexture(0, i).draw(Arg::topCenter = positionStart);
						}
					}

					const double positionStartY = static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(y + ry - updateInfo.currentPulse) * 480 / chartData.beat.resolution;
					if (positionStartY < 0)
					{
						// Laser point is above the drawing area
						break;
					}

					// Draw laser slam
					if (point.v != point.vf)
					{
						const Vec2 positionStart = {
							point.v * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
							static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(y + ry - updateInfo.currentPulse) * 480 / chartData.beat.resolution
						};

						const Vec2 positionEnd = {
							point.vf * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
							static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(y + ry - updateInfo.currentPulse) * 480 / chartData.beat.resolution
						};

						const bool isLeftToRight = (point.v < point.vf);
						const Quad quad = LaserSlamLineQuad(positionStart, positionEnd);

						for (const auto& [renderTargetTexture, laserNoteTexture] : m_drawTexturePairs)
						{
							const ScopedRenderTarget2D renderTarget(renderTargetTexture);
							laserNoteTexture(kLaserTextureSize.x * laneIdx, 0, kLaserTextureSize).mirrored(isLeftToRight).drawAt(positionStart + Vec2{ 0.0, -kLaserLineWidth / 2 });
							laserNoteTexture(kLaserTextureSize.x * laneIdx, 0, kLaserTextureSize).mirrored(!isLeftToRight).flipped().drawAt(positionEnd + Vec2{ 0.0, -kLaserLineWidth / 2 });
							quad(laserNoteTexture(kLaserTextureSize.x * laneIdx, 0, 1, kLaserTextureSize.y)).draw();
						}
					}

					// Last laser point does not create laser line
					const auto nextItr = std::next(itr);
					if (nextItr == laserSection.points.end())
					{
						// If the final note is a laser slam, draw its extension
						if (point.v != point.vf)
						{
							const Vec2 positionStart = {
								point.vf * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
								static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(y + ry - updateInfo.currentPulse) * 480 / chartData.beat.resolution
							};
							const Quad quad = LaserLineQuad(positionStart + Vec2{ 0.0, -kLaserTextureSize.y }, positionStart + Vec2{ 0.0, -kLaserTextureSize.y - 80.0 });

							for (const auto& [renderTargetTexture, laserNoteTexture] : m_drawTexturePairs)
							{
								const ScopedRenderTarget2D renderTarget(renderTargetTexture);
								quad(laserNoteTexture(kLaserTextureSize.x * laneIdx, kLaserTextureSize.y - 1, kLaserTextureSize.x, 1)).draw();
							}
						}

						break;
					}

					// Draw laser line by two laser points
					{
						const auto& [nextRy, nextPoint] = *nextItr;
						if (y + nextRy < updateInfo.currentPulse - chartData.beat.resolution)
						{
							continue;
						}

						const Vec2 positionStart = {
							point.vf * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
							static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(y + ry - updateInfo.currentPulse) * 480 / chartData.beat.resolution
						};

						const Vec2 positionEnd = {
							nextPoint.v * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
							static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(y + nextRy - updateInfo.currentPulse) * 480 / chartData.beat.resolution
						};

						const Quad quad = LaserLineQuad(positionStart + ((point.v != point.vf) ? Vec2{ 0.0, -kLaserTextureSize.y } : Vec2::Zero()), positionEnd);

						for (const auto& [renderTargetTexture, laserNoteTexture] : m_drawTexturePairs)
						{
							const ScopedRenderTarget2D renderTarget(renderTargetTexture);
							quad(laserNoteTexture(kLaserTextureSize.x * laneIdx, kLaserTextureSize.y - 1, kLaserTextureSize.x, 1)).draw();
						}
					}
				}
			}
		}
	}

	{
		const double highwayTiltRadians = m_highwayTilt.radians();
		Mat4x4 m = Mat4x4::Rotate(Vec3::Forward(), highwayTiltRadians, Vec3{ 0.0, 42.0, 0.0 });
		Transformer3D transform{ m };

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
