#include "laser_note_graphics.hpp"
#include "note_graphics_utils.hpp"
#include "music_game/graphics/graphics_defines.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kLaserNoteTextureFilename = U"laser.gif";
		constexpr StringView kLaserNoteMaskTextureFilename = U"laser_mask.gif";
		constexpr StringView kLaserNoteLeftStartTextureFilename = U"laserl_0.gif";
		constexpr StringView kLaserNoteRightStartTextureFilename = U"laserr_0.gif";

		constexpr Size kLaserTextureSize = { 48, 48 };
		constexpr double kLaserLineWidth = static_cast<double>(kLaserTextureSize.x);
		constexpr Size kLaserStartTextureSize = { 44, 200 };
		constexpr double kLaserShiftY = -10;

		enum : int32
		{
			kLaserNoteStartTextureColumnAdditive = 0,
			kLaserNoteStartTextureColumnInvMultiply,
		};

		Quad LaserLineQuad(const Vec2& positionStart, const Vec2& positionEnd)
		{
			return {
				positionStart + Vec2{ kLaserLineWidth / 2, 0.0 },
				positionStart + Vec2{ -kLaserLineWidth / 2, 0.0 },
				positionEnd + Vec2{ -kLaserLineWidth / 2, 0.0 },
				positionEnd + Vec2{ kLaserLineWidth / 2, 0.0 }
			};
		}

		void DrawLaserLine(int32 laneIdx, kson::RelPulse relPulse1, const kson::GraphValue& point1, kson::RelPulse relPulse2, const kson::GraphValue& point2, const Texture& laserNoteTexture)
		{
			const Vec2 positionStart = {
				point1.vf * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
				static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse1) * 480 / kson::kResolution + kLaserShiftY - ((point1.v != point1.vf) ? kLaserTextureSize.y : 0.0)
			};

			const Vec2 positionEnd = {
				point2.v * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
				static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse2) * 480 / kson::kResolution + kLaserShiftY
			};

			const Quad quad = LaserLineQuad(positionStart, positionEnd);
			quad(laserNoteTexture(kLaserTextureSize.x * laneIdx, kLaserTextureSize.y - 1 + kOnePixelTextureSourceOffset, kLaserTextureSize.x, kOnePixelTextureSourceSize)).draw();
		}

		Quad LaserSlamLineQuad(const Vec2& positionStart, const Vec2& positionEnd)
		{
			if (Abs(positionEnd.x - positionStart.x) <= kLaserLineWidth)
			{
				// Too short to draw line
				return { Vec2::Zero(), Vec2::Zero(), Vec2::Zero(), Vec2::Zero() };
			}

			const int diffXSign = Sign(positionEnd.x - positionStart.x);
			return {
				positionStart + Vec2{ diffXSign * kLaserLineWidth / 2, -kLaserLineWidth },
				positionEnd + Vec2{ -diffXSign * kLaserLineWidth / 2, -kLaserLineWidth },
				positionEnd + Vec2{ -diffXSign * kLaserLineWidth / 2, 0.0 },
				positionStart + Vec2{ diffXSign * kLaserLineWidth / 2, 0.0 }
			};
		}

		void DrawLaserSlam(int32 laneIdx, kson::RelPulse relPulse, const kson::GraphValue& point, const Texture& laserNoteTexture)
		{
			const Vec2 positionStart = {
				point.v * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
				static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse) * 480 / kson::kResolution + kLaserShiftY
			};

			const Vec2 positionEnd = {
				point.vf * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
				static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse) * 480 / kson::kResolution + kLaserShiftY
			};

			// Draw angles
			const bool isLeftToRight = (point.v < point.vf);
			laserNoteTexture(kLaserTextureSize.x * laneIdx, 0, kLaserTextureSize).mirrored(isLeftToRight).drawAt(positionStart + Vec2{ 0.0, -kLaserLineWidth / 2 });
			laserNoteTexture(kLaserTextureSize.x * laneIdx, 0, kLaserTextureSize).mirrored(!isLeftToRight).flipped().drawAt(positionEnd + Vec2{ 0.0, -kLaserLineWidth / 2 });

			// Draw line
			const Quad quad = LaserSlamLineQuad(positionStart, positionEnd);
			quad(laserNoteTexture(kLaserTextureSize.x * laneIdx + kOnePixelTextureSourceOffset, 0, kOnePixelTextureSourceSize, kLaserTextureSize.y)).draw();
		}

		void DrawLaserSlamTail(int32 laneIdx, kson::RelPulse relPulse, const kson::GraphValue& point, const Texture& laserNoteTexture)
		{
			const Vec2 positionStart = {
				point.vf * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
				static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse) * 480 / kson::kResolution + kLaserShiftY
			};
			const Quad quad = LaserLineQuad(positionStart + Vec2{ 0.0, -kLaserTextureSize.y }, positionStart + Vec2{ 0.0, -kLaserTextureSize.y - 80.0 });
			quad(laserNoteTexture(kLaserTextureSize.x * laneIdx, kLaserTextureSize.y - 1 + kOnePixelTextureSourceOffset, kLaserTextureSize.x, kOnePixelTextureSourceSize)).draw();
		}

		void DrawLaserSection(int32 laneIdx, kson::RelPulse relPulse, const kson::LaserSection& laserSection, const RenderTexture& target, const Texture& laserNoteTexture, const TextureRegion& laserStartTexture)
		{
			const double positionSectionStartY = static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse) * 480 / kson::kResolution + kLaserShiftY;
			if (positionSectionStartY + kLaserStartTextureSize.y < 0)
			{
				// Laser section is above the drawing area
				return;
			}

			const ScopedRenderTarget2D renderTarget(target);

			for (auto itr = laserSection.v.begin(); itr != laserSection.v.end(); ++itr)
			{
				const auto& [ry, point] = *itr;

				// Draw laser start texture
				if (itr == laserSection.v.begin())
				{
					const Vec2 positionStart = {
						point.v * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
						static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse + ry) * 480 / kson::kResolution + kLaserShiftY
					};
					laserStartTexture.draw(Arg::topCenter = positionStart);
				}

				const double positionStartY = static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse + ry) * 480 / kson::kResolution + kLaserShiftY;
				if (positionStartY < 0)
				{
					// Laser point is above the drawing area
					return;
				}

				// Draw laser slam
				if (point.v != point.vf)
				{
					DrawLaserSlam(laneIdx, relPulse + ry, point, laserNoteTexture);
				}

				// Last laser point does not create laser line
				const auto nextItr = std::next(itr);
				if (nextItr == laserSection.v.end())
				{
					// If the last point is a laser slam, draw its tail
					if (point.v != point.vf)
					{
						DrawLaserSlamTail(laneIdx, relPulse + ry, point, laserNoteTexture);
					}

					break;
				}

				// Draw laser line by two laser points
				{
					const auto& [nextRy, nextPoint] = *nextItr;
					if (relPulse + nextRy < -kson::kResolution)
					{
						continue;
					}
					DrawLaserLine(laneIdx, relPulse + ry, point, relPulse + nextRy, nextPoint, laserNoteTexture);
				}
			}
		}
	}

	LaserNoteGraphics::LaserNoteGraphics()
		: m_laserNoteTexture(TextureAsset(kLaserNoteTextureFilename))
		, m_laserNoteMaskTexture(TextureAsset(kLaserNoteMaskTextureFilename))
		, m_laserNoteStartTextures{
			TiledTexture(
				kLaserNoteLeftStartTextureFilename,
				{
					.column = 2,
					.sourceSize = kLaserStartTextureSize,
				}),
			TiledTexture(
				kLaserNoteRightStartTextureFilename,
				{
					.column = 2,
					.sourceSize = kLaserStartTextureSize,
				}) }
	{
	}

	void LaserNoteGraphics::draw(const kson::ChartData& chartData, const GameStatus& gameStatus, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const
	{
		const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);
		const ScopedRenderStates2D renderState(BlendState::Additive);

		// Draw laser notes
		for (int32 laneIdx = 0; laneIdx < kson::kNumLaserLanes; ++laneIdx) // Note: Use int32 instead of size_t here to avoid extra casting
		{
			const auto& lane = chartData.note.laser[laneIdx];
			for (const auto& [y, laserSection] : lane)
			{
				const kson::RelPulse relPulse = y - gameStatus.currentPulse;
				DrawLaserSection(laneIdx, relPulse, laserSection, additiveTarget, m_laserNoteTexture, m_laserNoteStartTextures[laneIdx](0, kLaserNoteStartTextureColumnAdditive));
				DrawLaserSection(laneIdx, relPulse, laserSection, invMultiplyTarget, m_laserNoteMaskTexture, m_laserNoteStartTextures[laneIdx](0, kLaserNoteStartTextureColumnInvMultiply));
			}
		}
	}
}
